function GenVectorMathTestData

    order = 1024;
    freqDomainLength = 513; % Number of complex numbers in frequency domain representation
    numTestFrames = 80;
    inputAudio = 'input.wav';
    startingSample = 80000;
    expectedFs = 48000;
    outputFilename = 'vectormath_test_data.h';
    
    % Preamble
    fid = fopen(outputFilename, 'w');
    if(fid < 0)
        error (['Failed to open ' outputFilename ' for writing.']);
    end
    fprintf(fid, '/* Copyright (c) Microsoft Corp. */\r\n\r\n');
    fprintf(fid, 'namespace VectorMathMatlabReference\r\n{\r\n\r\n');
    fprintf(fid, 'static const unsigned int order = %d;\r\n', order);
    fprintf(fid, 'static const unsigned int freqDomainLength = %d;\r\n', freqDomainLength);
    fprintf(fid, 'static const unsigned int numTestFrames = %d;\r\n', numTestFrames);
    
    % Get time domain test data, compute FFT, write reference
    [x, Fs] = audioread(inputAudio);
    if(Fs ~= expectedFs)
        error(['Input test data must have sample rate of ' num2str(expectedFs)]);
    end
    timeDomainData = x(startingSample:startingSample+numTestFrames*order-1);
    WriteVector(fid, timeDomainData, 'timeDomainReference', 'float');
    freqData = fft(reshape(timeDomainData, order, numTestFrames));
    freqData = freqData(1:513, :);
    freqData = freqData(:);
    freqData = [real(freqData) imag(freqData)]';
    freqData = freqData(:);
    WriteVector(fid, freqData, 'freqDomainReference', 'float');

    % Write test data for other math routines
    dataSet2 = x(startingSample+numTestFrames*order:startingSample+numTestFrames*2*order-1);
    WriteVector(fid, dataSet2, 'referenceData2', 'float');
    compData1 = timeDomainData(1:2:end) + 1i*timeDomainData(2:2:end);
    compData2 = dataSet2(1:2:end) + 1i*dataSet2(2:2:end);
    compMultiply = compData1.*compData2;
    compMultiply = [real(compMultiply) imag(compMultiply)]';
    compMultiply = compMultiply(:);
    WriteVector(fid, compMultiply, 'complexMultiplyReference', 'float');
    realMultiply = timeDomainData.*dataSet2;
    WriteVector(fid, realMultiply, 'realMultiplyReference', 'float');
    multConst = randn(1);
    fprintf(fid, 'static const float multiplyConstant = %ef;\r\n', multConst);
    constMultiply = multConst*timeDomainData;
    WriteVector(fid, constMultiply, 'constMultiplyReference', 'float');
    dataSet3 = x(startingSample+numTestFrames*order*2:startingSample+numTestFrames*order*3-1);
    WriteVector(fid, dataSet3, 'referenceData3', 'float');
    realMac = timeDomainData + dataSet2 .* dataSet3;
    WriteVector(fid, realMac, 'realMacReference', 'float');
    compData3 = dataSet3(1:2:end) + 1i*dataSet3(2:2:end);
    compMac = compData1 + compData2 .* compData3;
    compMac = [real(compMac) imag(compMac)]';
    compMac = compMac(:);
    WriteVector(fid, compMac, 'complexMacReference', 'float');
    realSum = timeDomainData + dataSet2;
    WriteVector(fid, realSum, 'realSumReference', 'float');
    realDotProd = sum(reshape(timeDomainData, order, numTestFrames).*reshape(dataSet2, order, numTestFrames))';
    WriteVector(fid, realDotProd, 'realDotProdReference', 'float');
    addProductConst = randn(1);
    fprintf(fid, 'static const float addProductConst = %ef;\r\n', addProductConst);
    addProductConstResult = timeDomainData + dataSet2 * addProductConst;
    WriteVector(fid, addProductConstResult, 'addProductConstResult', 'float');
    
    % Postamble
    fprintf(fid, '} // namespace VectorMathMatlabReference\r\n\r\n');
    fclose(fid);
end

function WriteVector(fid, data, variableName, variableType)
    fprintf(fid, 'static const %s %s[] = {\r\n', variableType, variableName);
    for i = 1:size(data,1)-1
        fprintf(fid, '%ef,\r\n', data(i,1));
    end
    fprintf(fid, '%ef};\r\n\r\n', data(end,1));
end
