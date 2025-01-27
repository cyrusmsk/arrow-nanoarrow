// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include <array>
#include <cerrno>

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "nanoarrow/nanoarrow_gtest_util.hpp"
#include "nanoarrow/nanoarrow_testing.hpp"

using testing::ElementsAre;

TEST(NanoarrowHppTest, NanoarrowHppExceptionTest) {
  ASSERT_THROW(NANOARROW_THROW_NOT_OK(EINVAL), nanoarrow::Exception);
  ASSERT_NO_THROW(NANOARROW_THROW_NOT_OK(NANOARROW_OK));
  try {
    NANOARROW_THROW_NOT_OK(EINVAL);
  } catch (const nanoarrow::Exception& e) {
    EXPECT_EQ(std::string(e.what()).substr(0, 24), "EINVAL failed with errno");
  }
}

TEST(NanoarrowHppTest, NanoarrowHppUniqueArrayTest) {
  nanoarrow::UniqueArray array;
  EXPECT_EQ(array->release, nullptr);

  ASSERT_EQ(ArrowArrayInitFromType(array.get(), NANOARROW_TYPE_INT32), NANOARROW_OK);
  ASSERT_EQ(ArrowArrayStartAppending(array.get()), NANOARROW_OK);
  ASSERT_EQ(ArrowArrayAppendInt(array.get(), 123), NANOARROW_OK);
  ASSERT_EQ(ArrowArrayFinishBuildingDefault(array.get(), nullptr), NANOARROW_OK);

  EXPECT_NE(array->release, nullptr);
  EXPECT_EQ(array->length, 1);

  // move constructor
  nanoarrow::UniqueArray array2 = std::move(array);
  EXPECT_EQ(array->release, nullptr);  // NOLINT(clang-analyzer-cplusplus.Move)
  EXPECT_NE(array2->release, nullptr);
  EXPECT_EQ(array2->length, 1);

  // pointer constructor
  nanoarrow::UniqueArray array3(array2.get());
  EXPECT_EQ(array2->release, nullptr);
  EXPECT_NE(array3->release, nullptr);
  EXPECT_EQ(array3->length, 1);
}

TEST(NanoarrowHppTest, NanoarrowHppUniqueSchemaTest) {
  nanoarrow::UniqueSchema schema;
  EXPECT_EQ(schema->release, nullptr);

  ASSERT_EQ(ArrowSchemaInitFromType(schema.get(), NANOARROW_TYPE_INT32), NANOARROW_OK);
  EXPECT_NE(schema->release, nullptr);
  EXPECT_STREQ(schema->format, "i");

  // move constructor
  nanoarrow::UniqueSchema schema2 = std::move(schema);
  EXPECT_EQ(schema->release, nullptr);  // NOLINT(clang-analyzer-cplusplus.Move)
  EXPECT_NE(schema2->release, nullptr);
  EXPECT_STREQ(schema2->format, "i");

  // pointer constructor
  nanoarrow::UniqueSchema schema3(schema2.get());
  EXPECT_EQ(schema2->release, nullptr);
  EXPECT_NE(schema3->release, nullptr);
  EXPECT_STREQ(schema3->format, "i");
}

TEST(NanoarrowHppTest, NanoarrowHppUniqueArrayStreamTest) {
  nanoarrow::UniqueSchema schema;
  schema->format = NULL;

  nanoarrow::UniqueArrayStream array_stream_default;
  EXPECT_EQ(array_stream_default->release, nullptr);

  nanoarrow::UniqueSchema schema_in;
  EXPECT_EQ(ArrowSchemaInitFromType(schema_in.get(), NANOARROW_TYPE_INT32), NANOARROW_OK);
  auto array_stream = nanoarrow::EmptyArrayStream::MakeUnique(schema_in.get());
  EXPECT_NE(array_stream->release, nullptr);
  EXPECT_EQ(ArrowArrayStreamGetSchema(array_stream.get(), schema.get(), nullptr),
            NANOARROW_OK);
  EXPECT_STREQ(schema->format, "i");
  schema.reset();
  schema->format = NULL;

  // move constructor
  nanoarrow::UniqueArrayStream array_stream2 = std::move(array_stream);
  EXPECT_EQ(array_stream->release, nullptr);  // NOLINT(clang-analyzer-cplusplus.Move)
  EXPECT_NE(array_stream2->release, nullptr);
  EXPECT_EQ(ArrowArrayStreamGetSchema(array_stream2.get(), schema.get(), nullptr),
            NANOARROW_OK);
  EXPECT_STREQ(schema->format, "i");
  schema.reset();
  schema->format = NULL;

  // pointer constructor
  nanoarrow::UniqueArrayStream array_stream3(array_stream2.get());
  EXPECT_EQ(array_stream2->release, nullptr);
  EXPECT_NE(array_stream3->release, nullptr);
  EXPECT_EQ(ArrowArrayStreamGetSchema(array_stream2.get(), schema.get(), nullptr),
            NANOARROW_OK);
  EXPECT_STREQ(schema->format, "i");

  // releasing should clear the release callback
  EXPECT_EQ(ArrowSchemaInitFromType(schema_in.get(), NANOARROW_TYPE_INT32), NANOARROW_OK);
  auto array_stream4 = nanoarrow::EmptyArrayStream::MakeUnique(schema_in.get());
  EXPECT_NE(array_stream4->release, nullptr);
  array_stream4->release(array_stream4.get());
  EXPECT_EQ(array_stream4->private_data, nullptr);
  EXPECT_EQ(array_stream4->release, nullptr);
}

TEST(NanoarrowHppTest, NanoarrowHppUniqueBufferTest) {
  nanoarrow::UniqueBuffer buffer;
  EXPECT_EQ(buffer->data, nullptr);
  EXPECT_EQ(buffer->size_bytes, 0);

  ASSERT_EQ(ArrowBufferAppendFill(buffer.get(), 0xff, 123), NANOARROW_OK);
  EXPECT_NE(buffer->data, nullptr);
  EXPECT_EQ(buffer->size_bytes, 123);

  // move constructor
  nanoarrow::UniqueBuffer buffer2 = std::move(buffer);
  EXPECT_EQ(buffer->data, nullptr);  // NOLINT(clang-analyzer-cplusplus.Move)
  EXPECT_EQ(buffer->size_bytes, 0);  // NOLINT(clang-analyzer-cplusplus.Move)
  EXPECT_NE(buffer2->data, nullptr);
  EXPECT_EQ(buffer2->size_bytes, 123);

  // pointer constructor
  nanoarrow::UniqueBuffer buffer3(buffer2.get());
  EXPECT_EQ(buffer2->data, nullptr);
  EXPECT_EQ(buffer2->size_bytes, 0);
  EXPECT_NE(buffer3->data, nullptr);
  EXPECT_EQ(buffer3->size_bytes, 123);
}

TEST(NanoarrowHppTest, NanoarrowHppUniqueBitmapTest) {
  nanoarrow::UniqueBitmap bitmap;
  EXPECT_EQ(bitmap->buffer.data, nullptr);
  EXPECT_EQ(bitmap->size_bits, 0);

  ASSERT_EQ(ArrowBitmapAppend(bitmap.get(), true, 123), NANOARROW_OK);
  EXPECT_NE(bitmap->buffer.data, nullptr);
  EXPECT_EQ(bitmap->size_bits, 123);

  // move constructor
  nanoarrow::UniqueBitmap bitmap2 = std::move(bitmap);
  EXPECT_EQ(bitmap->buffer.data, nullptr);  // NOLINT(clang-analyzer-cplusplus.Move)
  EXPECT_EQ(bitmap->size_bits, 0);          // NOLINT(clang-analyzer-cplusplus.Move)
  EXPECT_NE(bitmap2->buffer.data, nullptr);
  EXPECT_EQ(bitmap2->size_bits, 123);

  // pointer constructor
  nanoarrow::UniqueBitmap bitmap3(bitmap2.get());
  EXPECT_EQ(bitmap2->buffer.data, nullptr);
  EXPECT_EQ(bitmap2->size_bits, 0);
  EXPECT_NE(bitmap3->buffer.data, nullptr);
  EXPECT_EQ(bitmap3->size_bits, 123);
}

struct TestWrappedObj {
  int64_t* num_frees;

  TestWrappedObj(int64_t* addr) { num_frees = addr; }

  TestWrappedObj(TestWrappedObj&& obj) {
    num_frees = obj.num_frees;
    obj.num_frees = nullptr;
  }

  ~TestWrappedObj() {
    if (num_frees != nullptr) {
      *num_frees = *num_frees + 1;
    }
  }
};

TEST(NanoarrowHppTest, NanoarrowHppBufferInitWrappedTest) {
  nanoarrow::UniqueBuffer buffer;
  int64_t num_frees = 0;

  TestWrappedObj obj(&num_frees);
  nanoarrow::BufferInitWrapped(buffer.get(), std::move(obj), nullptr, 0);
  EXPECT_EQ(obj.num_frees, nullptr);
  EXPECT_EQ(num_frees, 0);
  buffer.reset();
  EXPECT_EQ(num_frees, 1);

  // Ensure the destructor won't get called again when ArrowBufferReset is
  // called on the empty buffer.
  buffer.reset();
  EXPECT_EQ(num_frees, 1);
}

TEST(NanoarrowHppTest, NanoarrowHppBufferInitSequenceTest) {
  nanoarrow::UniqueBuffer buffer;

  // Check templating magic with std::string
  nanoarrow::BufferInitSequence(buffer.get(), std::string("1234"));
  EXPECT_EQ(buffer->size_bytes, 4);
  EXPECT_EQ(buffer->capacity_bytes, 0);
  EXPECT_EQ(memcmp(buffer->data, "1234", 4), 0);

  // Check templating magic with std::vector
  buffer.reset();
  nanoarrow::BufferInitSequence(buffer.get(), std::vector<uint8_t>({1, 2, 3, 4}));
  EXPECT_EQ(buffer->size_bytes, 4);
  EXPECT_EQ(buffer->capacity_bytes, 0);
  EXPECT_EQ(buffer->data[0], 1);
  EXPECT_EQ(buffer->data[1], 2);
  EXPECT_EQ(buffer->data[2], 3);
  EXPECT_EQ(buffer->data[3], 4);

  // Check templating magic with std::array
  buffer.reset();
  nanoarrow::BufferInitSequence(buffer.get(), std::array<uint8_t, 4>({1, 2, 3, 4}));
  EXPECT_EQ(buffer->size_bytes, 4);
  EXPECT_EQ(buffer->capacity_bytes, 0);
  EXPECT_EQ(buffer->data[0], 1);
  EXPECT_EQ(buffer->data[1], 2);
  EXPECT_EQ(buffer->data[2], 3);
  EXPECT_EQ(buffer->data[3], 4);
}

TEST(NanoarrowHppTest, NanoarrowHppUniqueArrayViewTest) {
  nanoarrow::UniqueArrayView array_view;
  EXPECT_EQ(array_view->storage_type, NANOARROW_TYPE_UNINITIALIZED);

  // Use an ArrayView with children, since an ArrayView with no children
  // doesn't hold any resources
  ArrowArrayViewInitFromType(array_view.get(), NANOARROW_TYPE_STRUCT);
  ASSERT_EQ(ArrowArrayViewAllocateChildren(array_view.get(), 2), NANOARROW_OK);
  EXPECT_EQ(array_view->storage_type, NANOARROW_TYPE_STRUCT);

  // move constructor
  nanoarrow::UniqueArrayView array_view2 = std::move(array_view);
  EXPECT_EQ(array_view->storage_type,  // NOLINT(clang-analyzer-cplusplus.Move)
            NANOARROW_TYPE_UNINITIALIZED);
  EXPECT_EQ(array_view2->storage_type, NANOARROW_TYPE_STRUCT);

  // pointer constructor
  nanoarrow::UniqueArrayView array_view3(array_view2.get());
  EXPECT_EQ(array_view2->storage_type, NANOARROW_TYPE_UNINITIALIZED);
  EXPECT_EQ(array_view3->storage_type, NANOARROW_TYPE_STRUCT);
}

TEST(NanoarrowHppTest, NanoarrowHppViewArrayAsTest) {
  nanoarrow::UniqueBuffer is_valid, floats;
  nanoarrow::BufferInitSequence(is_valid.get(), std::vector<uint8_t>{0xFF});
  ArrowBitClear(is_valid->data, 2);
  ArrowBitClear(is_valid->data, 5);
  nanoarrow::BufferInitSequence(floats.get(),
                                std::vector<float>{8, 4, 2, 1, .5, .25, .125});

  const void* buffers[] = {is_valid->data, floats->data};
  struct ArrowArray array {};
  array.length = 7;
  array.null_count = 2;
  array.n_buffers = 2;
  array.buffers = buffers;

  int i = 0;
  float f = 8;
  for (auto slot : nanoarrow::ViewArrayAs<float>(&array)) {
    if (i == 2 || i == 5) {
      EXPECT_EQ(slot, nanoarrow::NA);
    } else {
      EXPECT_EQ(slot, f);
    }
    ++i;
    f /= 2;
  }
}

TEST(NanoarrowHppTest, NanoarrowHppViewArrayAsBytesTest) {
  using namespace nanoarrow::literals;

  nanoarrow::UniqueBuffer is_valid, offsets, data;
  nanoarrow::BufferInitSequence(is_valid.get(), std::vector<uint8_t>{0xFF});
  ArrowBitClear(is_valid->data, 2);
  ArrowBitClear(is_valid->data, 5);
  nanoarrow::BufferInitSequence(offsets.get(),
                                std::vector<int32_t>{0, 1, 2, 3, 4, 5, 6, 7});
  nanoarrow::BufferInitSequence(data.get(), std::string{"abcdefghi"});

  const void* buffers[] = {is_valid->data, offsets->data, data->data};
  struct ArrowArray array {};
  array.length = 7;
  array.null_count = 2;
  array.n_buffers = 2;
  array.buffers = buffers;

  int i = 0;
  ArrowStringView expected[] = {"a"_asv, "b"_asv, "c"_asv, "d"_asv,
                                "e"_asv, "f"_asv, "g"_asv};
  for (auto slot : nanoarrow::ViewArrayAsBytes<32>(&array)) {
    if (i == 2 || i == 5) {
      EXPECT_EQ(slot, nanoarrow::NA);
    } else {
      EXPECT_EQ(slot, expected[i]);
    }
    ++i;
  }
}

TEST(NanoarrowHppTest, NanoarrowHppViewArrayAsFixedSizeBytesTest) {
  using namespace nanoarrow::literals;

  nanoarrow::UniqueBuffer is_valid, data;
  nanoarrow::BufferInitSequence(is_valid.get(), std::vector<uint8_t>{0xFF});
  ArrowBitClear(is_valid->data, 2);
  ArrowBitClear(is_valid->data, 5);
  nanoarrow::BufferInitSequence(
      data.get(), std::string{"foo"} + "bar" + "foo" + "bar" + "foo" + "bar" + "foo");

  const void* buffers[] = {is_valid->data, data->data};
  struct ArrowArray array {};
  array.length = 7;
  array.null_count = 2;
  array.n_buffers = 2;
  array.buffers = buffers;

  int i = 0;
  for (auto slot : nanoarrow::ViewArrayAsFixedSizeBytes(&array, 3)) {
    if (i == 2 || i == 5) {
      EXPECT_EQ(slot, nanoarrow::NA);
    } else {
      EXPECT_EQ(slot, i % 2 == 0 ? "foo"_asv : "bar"_asv);
    }
    ++i;
  }
}

TEST(NanoarrowHppTest, NanoarrowHppViewArrayStreamTest) {
  static int32_t slot = 1;

  struct ArrowArrayStream stream {};
  stream.get_schema = [](struct ArrowArrayStream*, struct ArrowSchema* out) {
    return ArrowSchemaInitFromType(out, NANOARROW_TYPE_INT32);
  };
  stream.get_next = [](struct ArrowArrayStream*, struct ArrowArray* out) {
    if (slot >= 16) return ENOMEM;
    NANOARROW_RETURN_NOT_OK(ArrowArrayInitFromType(out, NANOARROW_TYPE_INT32));
    NANOARROW_RETURN_NOT_OK(ArrowArrayStartAppending(out));
    NANOARROW_RETURN_NOT_OK(ArrowArrayAppendInt(out, slot *= 2));
    return ArrowArrayFinishBuildingDefault(out, nullptr);
  };
  stream.get_last_error = [](struct ArrowArrayStream*) { return "foo bar"; };
  stream.release = [](struct ArrowArrayStream*) {};

  nanoarrow::ViewArrayStream stream_view(&stream);
  for (ArrowArray& array : stream_view) {
    EXPECT_THAT(nanoarrow::ViewArrayAs<int32_t>(&array), ElementsAre(slot));
  }
  EXPECT_EQ(stream_view.count(), 4);
  EXPECT_EQ(stream_view.code(), ENOMEM);
  EXPECT_STREQ(stream_view.error()->message, "foo bar");
}

TEST(NanoarrowHppTest, NanoarrowHppEmptyArrayStreamTest) {
  nanoarrow::UniqueSchema schema;
  struct ArrowArray array;

  nanoarrow::UniqueSchema schema_in;
  EXPECT_EQ(ArrowSchemaInitFromType(schema_in.get(), NANOARROW_TYPE_INT32), NANOARROW_OK);

  nanoarrow::UniqueArrayStream array_stream;
  nanoarrow::EmptyArrayStream(schema_in.get()).ToArrayStream(array_stream.get());

  EXPECT_EQ(ArrowArrayStreamGetSchema(array_stream.get(), schema.get(), nullptr),
            NANOARROW_OK);
  EXPECT_STREQ(schema->format, "i");
  EXPECT_EQ(ArrowArrayStreamGetNext(array_stream.get(), &array, nullptr), NANOARROW_OK);
  EXPECT_EQ(array.release, nullptr);
  EXPECT_STREQ(ArrowArrayStreamGetLastError(array_stream.get()), "");
}

TEST(NanoarrowHppTest, NanoarrowHppVectorArrayStreamTest) {
  nanoarrow::UniqueArray array_in;
  EXPECT_EQ(ArrowArrayInitFromType(array_in.get(), NANOARROW_TYPE_INT32), NANOARROW_OK);
  EXPECT_EQ(ArrowArrayStartAppending(array_in.get()), NANOARROW_OK);
  EXPECT_EQ(ArrowArrayAppendInt(array_in.get(), 1234), NANOARROW_OK);
  EXPECT_EQ(ArrowArrayFinishBuildingDefault(array_in.get(), nullptr), NANOARROW_OK);

  nanoarrow::UniqueSchema schema_in;
  EXPECT_EQ(ArrowSchemaInitFromType(schema_in.get(), NANOARROW_TYPE_INT32), NANOARROW_OK);

  nanoarrow::UniqueArrayStream array_stream;
  nanoarrow::VectorArrayStream(schema_in.get(), array_in.get())
      .ToArrayStream(array_stream.get());

  nanoarrow::ViewArrayStream array_stream_view(array_stream.get());
  for (ArrowArray& array : array_stream_view) {
    EXPECT_THAT(nanoarrow::ViewArrayAs<int32_t>(&array), ElementsAre(1234));
  }
  EXPECT_EQ(array_stream_view.count(), 1);
  EXPECT_EQ(array_stream_view.code(), NANOARROW_OK);
  EXPECT_STREQ(array_stream_view.error()->message, "");
}
