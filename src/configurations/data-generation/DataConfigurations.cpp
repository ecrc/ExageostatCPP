
/*
 * Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
 * Copyright (C) 2023 by Brightskies inc,
 * All rights reserved.
 * ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).
 */

/**
 * @file DataConfigurations.cpp
 * @brief This file contains the implementation of the DataConfigurations class.
 * @version 1.0.0
 * @author Sameh Abdulah
 * @date 2023-02-03
**/

#include <configurations/data-generation/DataConfigurations.hpp>
#include <algorithm>
#include <cstring>

using namespace exageostat::configurations::data_configurations;
using namespace std;
using namespace exageostat::common;

string DataConfigurations::GetKernel() {
    return this->mKernel;
}

void DataConfigurations::SetKernel(const std::string &aKernel) {
    this->mKernel = aKernel;
}

void DataConfigurations::SetIsSynthetic(bool aIsSynthetic) {
    this->mIsSynthetic = aIsSynthetic;
}

bool DataConfigurations::GetIsSynthetic() const {
    return this->mIsSynthetic;
}

void DataConfigurations::SetParametersNumber(int aParameterNumbers) {
    this->mParametersNumber = aParameterNumbers;
}

int DataConfigurations::GetParametersNumber() const {
    return this->mParametersNumber;
}

void DataConfigurations::SetLowerBounds(double *apTheta) {
    this->mLowerBounds = apTheta;
}

double *DataConfigurations::GetLowerBounds() {
    return this->mLowerBounds;
}

void DataConfigurations::SetUpperBounds(int aSize) {
    this->mUpperBounds = (double *) std::malloc(aSize * sizeof(double));
}

double *DataConfigurations::GetUpperBounds() {
    return this->mUpperBounds;
}

void DataConfigurations::CheckKernelValue(const string &aKernel) {

    // Check if the kernel name exists in the availableKernels set.
    if (availableKernels.count(aKernel) <= 0) {
        throw range_error("Invalid value for Kernel. Please check manual.");
    } else {
        // Check if the string is already in CamelCase format
        if (IsCamelCase(aKernel)) {
            this->SetKernel(aKernel);
            return;
        }
        string str = aKernel;
        // Replace underscores with spaces and split the string into words
        std::replace(str.begin(), str.end(), '_', ' ');
        std::istringstream iss(str);
        std::string word, result;
        while (iss >> word) {
            // Capitalize the first letter of each word and append it to the result
            word[0] = toupper(word[0]);
            result += word;
        }
        this->SetKernel(result);
    }
}

bool DataConfigurations::IsCamelCase(std::string aString) {
    // If the string contains an underscore, it is not in CamelCase format
    if (aString.find('_') != std::string::npos) {
        return false;
    }
    // If the string starts with a lowercase letter, it is not in CamelCase format
    if (islower(aString[0])) {
        return false;
    }
    // If none of the above conditions hold, the string is in CamelCase format
    return true;
}

double *DataConfigurations::ParseTheta(const std::string& aInputValues) {
    // Count the number of values in the string
    int num_values = 1;
    for (char aInputValue : aInputValues) {
        if (aInputValue == ':') {
            num_values++;
        }
    }

    // Allocate memory for the array of doubles
    auto* theta = (double*) malloc(num_values * sizeof(double));

    // Split the string into tokens using strtok()
    const char* delim = ":";
    char* token = strtok((char*)aInputValues.c_str(), delim);
    int i = 0;
    while (token != nullptr) {
        // Check if the token is a valid double or "?"
        if (!strcmp(token, "?")) {
            theta[i] = -1;
        }
        else {
            try {
                theta[i] = stod(token);
            }
            catch (...) {
                // If it's not a valid double or "?", throw an error
                free(theta);
                cout << "Error: " << token << " is not a valid double or '?' " << endl;
                throw range_error("Invalid value. Please use Numerical values only.");
            }
        }

        // Get the next token
        token = strtok(nullptr, delim);
        i++;
    }

    // Check if the number of values in the array is correct
    if (i != num_values) {
        free(theta);
        throw range_error("Error: the number of values in the input string is invalid, please use this example format as a reference 1:?:0.1");
    }

    return theta;
}
