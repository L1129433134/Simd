/*
* Simd Library Tests.
*
* Copyright (c) 2011-2013 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "Test/TestUtils.h"
#include "Test/TestPerformance.h"
#include "Test/Test.h"

namespace Test
{
    SIMD_INLINE std::string CompareTypeDescription(Simd::CompareType type)
    {
        switch(type)
        {
        case Simd::CompareGreaterThen:
            return "(>)";
        case Simd::CompareLesserThen:
            return "(<)";
        case Simd::CompareEqualTo:
            return "(=)";
        }
        assert(0);
        return "(Unknown)";
    }	
    
    namespace
	{
		struct Func1
		{
			typedef void (*FuncPtr)(const uchar * src, size_t srcStride, size_t width, size_t height, 
				uchar value, uchar positive, uchar negative, uchar * dst, size_t dstStride, Simd::CompareType type);

			FuncPtr func;
			std::string description;

			Func1(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

			void Call(const View & src, uchar value, uchar positive, uchar negative, View & dst, Simd::CompareType type) const
			{
				TEST_PERFORMANCE_TEST(description);
				func(src.data, src.stride, src.width, src.height, value, positive, negative, dst.data, dst.stride, type);
			}
		};
	}

#define ARGS11(width, height, type, function1, function2) \
    width, height, type, \
    Func1(function1.func, function1.description + CompareTypeDescription(type)), \
    Func1(function2.func, function2.description + CompareTypeDescription(type))

#define ARGS12(function1, function2) \
    Func1(function1, std::string(#function1)), Func1(function2, std::string(#function2))

	bool BinarizationTest(int width, int height, Simd::CompareType type, const Func1 & f1, const Func1 & f2)
	{
		bool result = true;

		std::cout << "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "]." << std::endl;

		View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(src);

		uchar value = Random(256);
		uchar positive = Random(256);
		uchar negative = Random(256);

		View d1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View d2(width, height, View::Gray8, NULL, TEST_ALIGN(width));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(src, value, positive, negative, d1, type));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(src, value, positive, negative, d2, type));

		result = result && Compare(d1, d2, 0, true, 10);

		return result;
	}

    bool BinarizationTest(const Func1 & f1, const Func1 & f2)
    {
        bool result = true;

        for(Simd::CompareType type = Simd::CompareGreaterThen; type <= Simd::CompareEqualTo && result; type = Simd::CompareType(type + 1))
        {
            result = result && BinarizationTest(ARGS11(W, H, type, f1, f2));
            result = result && BinarizationTest(ARGS11(W + 1, H - 1, type, f1, f2));
        }

        return result;
    }

	bool BinarizationTest()
	{
		bool result = true;

		result = result && BinarizationTest(ARGS12(Simd::Base::Binarization, Simd::Binarization));

#if defined(SIMD_SSE2_ENABLE) && defined(SIMD_AVX2_ENABLE)
        if(Simd::Sse2::Enable && Simd::Avx2::Enable)
            result = result && BinarizationTest(ARGS12(Simd::Avx2::Binarization, Simd::Sse2::Binarization));
#endif 

		return result;
	}

    namespace
    {
        struct Func2
        {
            typedef void (*FuncPtr)(const uchar * src, size_t srcStride, size_t width, size_t height, 
                uchar value, size_t neighborhood, uchar threshold, uchar positive, uchar negative, 
                uchar * dst, size_t dstStride, Simd::CompareType type);

            FuncPtr func;
            std::string description;

            Func2(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

            void Call(const View & src, uchar value, size_t neighborhood, uchar threshold, uchar positive, uchar negative, View & dst, Simd::CompareType type) const
            {
                TEST_PERFORMANCE_TEST(description);
                func(src.data, src.stride, src.width, src.height, value, neighborhood, threshold, positive, negative, dst.data, dst.stride, type);
            }
        };
    }

#define ARGS21(width, height, type, function1, function2) \
    width, height, type, \
    Func2(function1.func, function1.description + CompareTypeDescription(type)), \
    Func2(function2.func, function2.description + CompareTypeDescription(type))

#define ARGS22(function1, function2) \
    Func2(function1, std::string(#function1)), Func2(function2, std::string(#function2))

    bool AveragingBinarizationTest(int width, int height, Simd::CompareType type, const Func2 & f1, const Func2 & f2)
    {
        bool result = true;

        std::cout << "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "]." << std::endl;

        View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        FillRandom(src);

        uchar value = 127;
        size_t neighborhood = 17;
        uchar threshold = 128;
        uchar positive = 7;
        uchar negative = 3;

        View d1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View d2(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(src, value, neighborhood, threshold, positive, negative, d1, type));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(src, value, neighborhood, threshold, positive, negative, d2, type));

        result = result && Compare(d1, d2, 0, true, 64);

        return result;
    }

    bool AveragingBinarizationTest(const Func2 & f1, const Func2 & f2)
    {
        bool result = true;

        for(Simd::CompareType type = Simd::CompareGreaterThen; type < Simd::CompareEqualTo && result; type = Simd::CompareType(type + 1))
        {
            result = result && AveragingBinarizationTest(ARGS21(W, H, type, f1, f2));
            result = result && AveragingBinarizationTest(ARGS21(W + 1, H - 1, type, f1, f2));
        }

        return result;
    }

    bool AveragingBinarizationTest()
    {
        bool result = true;

        result = result && AveragingBinarizationTest(ARGS22(Simd::Base::AveragingBinarization, Simd::AveragingBinarization));

#if defined(SIMD_SSE2_ENABLE) && defined(SIMD_AVX2_ENABLE)
        if(Simd::Sse2::Enable && Simd::Avx2::Enable)
            result = result && AveragingBinarizationTest(ARGS22(Simd::Avx2::AveragingBinarization, Simd::Sse2::AveragingBinarization));
#endif 

        return result;
    }
}