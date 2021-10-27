// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "gtest.h"
#include "VectorMath.h"
#include "vectormath_generic.h"
#include "vectormath_test_data.h"
#include "cputype.h"
#include "AlignedAllocator.h"  // AlignedStore
#include "CommonTestHelpers.h" // FloatsTooFarApart
#include <memory>              // std::unique_ptr
#include <fstream>             // wofstream
#include <thread>              // threads

// When debugging, uncomment the following line to save results to a file
// #define SAVE_FILTER_OUTPUT

const int NUM_THREADS = 4;
namespace AudioUnitTests
{

    class CVectorMathTests : public ::testing::Test
    {
    protected:
        CVectorMathTests();

        virtual ~CVectorMathTests();

        void SetUp();

        void TearDown();

    protected:
        AlignedStore::aligned_vector<float> m_TimeDomainAlignedBuffer;
        AlignedStore::aligned_vector<VectorMath::floatFC> m_FreqDomainAlignedBuffer;
    };

    CVectorMathTests::CVectorMathTests()
    {
    }

    CVectorMathTests::~CVectorMathTests()
    {
    }

    void CVectorMathTests::SetUp()
    {
        m_TimeDomainAlignedBuffer.resize(VectorMathMatlabReference::order);
        m_FreqDomainAlignedBuffer.resize(VectorMathMatlabReference::freqDomainLength);
    }

    void CVectorMathTests::TearDown()
    {
    }

    TEST_F(CVectorMathTests, TestOptimizedVectorMathRoutines)
    {
        unsigned int numFrames = VectorMathMatlabReference::numTestFrames;
        unsigned int frameLength = VectorMathMatlabReference::order;
        unsigned int numMismatchedSamples = 0;
        float percentMismatched = 0.0f;
        AlignedStore::aligned_vector<float> RealVec1(VectorMathMatlabReference::order);
        AlignedStore::aligned_vector<float> RealVec2(VectorMathMatlabReference::order);

        // Test real dot product
        for (unsigned int i = 0; i < numFrames; i++)
        {
            memcpy(
                RealVec1.data(),
                &VectorMathMatlabReference::timeDomainReference[i * frameLength],
                frameLength * sizeof(float));
            memcpy(
                RealVec2.data(),
                &VectorMathMatlabReference::referenceData2[i * frameLength],
                frameLength * sizeof(float));
            float result = 0;
            EXPECT_NO_FATAL_FAILURE(
                VectorMath::Arithmetic::DotProd_32f(&result, RealVec1.data(), RealVec2.data(), frameLength));
            if (AreFloatsTooFarApart(result, VectorMathMatlabReference::realDotProdReference[i]))
            {
                numMismatchedSamples++;
            }
        }
        percentMismatched = 100.0f * static_cast<float>(numMismatchedSamples) / static_cast<float>(numFrames);
        EXPECT_TRUE(percentMismatched < 0.5);

        numMismatchedSamples = 0;
        // Test add product const
        for (unsigned int i = 0; i < numFrames; i++)
        {
            memcpy(
                RealVec1.data(),
                &VectorMathMatlabReference::timeDomainReference[i * frameLength],
                frameLength * sizeof(float));
            memcpy(
                RealVec2.data(),
                &VectorMathMatlabReference::referenceData2[i * frameLength],
                frameLength * sizeof(float));
            EXPECT_NO_FATAL_FAILURE(VectorMath::Arithmetic::AddProductC_32f(
                RealVec1.data(), RealVec2.data(), VectorMathMatlabReference::addProductConst, frameLength));

            for (unsigned int j = 0; j < frameLength; j++)
            {
                if (AreFloatsTooFarApart(
                        RealVec1[j], VectorMathMatlabReference::addProductConstResult[i * frameLength + j]))
                {
                    numMismatchedSamples++;
                }
            }
        }
        percentMismatched =
            100.0f * static_cast<float>(numMismatchedSamples) / static_cast<float>(frameLength * numFrames);
        EXPECT_TRUE(percentMismatched < 0.5);
    }
}; // namespace AudioUnitTests
