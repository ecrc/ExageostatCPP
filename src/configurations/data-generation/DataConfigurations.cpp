
/*
 * Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
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

#include <algorithm>
#include <cstring>
#include <utility>
#include <vector>

#include <configurations/data-generation/DataConfigurations.hpp>

using namespace std;

using namespace exageostat::configurations::data_configurations;
using namespace exageostat::common;

string DataConfigurations::GetKernel() const{
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

void DataConfigurations::SetLowerBounds(std::vector<double> &apTheta) {
    this->mLowerBounds = apTheta;
}

std::vector<double> &DataConfigurations::GetLowerBounds() {
    return this->mLowerBounds;
}

void DataConfigurations::SetUpperBounds(std::vector<double> &apTheta) {
    this->mUpperBounds = apTheta;
}

std::vector<double> &DataConfigurations::GetUpperBounds() {
    return this->mUpperBounds;
}

void DataConfigurations::SetStartingTheta(std::vector<double> &apTheta) {
    this->mStartingTheta = apTheta;
}

std::vector<double> &DataConfigurations::GetStartingTheta() {
    return this->mStartingTheta;
}

void DataConfigurations::SetInitialTheta(std::vector<double> &apTheta) {
    this->mInitialTheta = apTheta;
}

std::vector<double> &DataConfigurations::GetInitialTheta() {
    return this->mInitialTheta;
}

void DataConfigurations::SetTargetTheta(std::vector<double> &apTheta) {
    this->mTargetTheta = apTheta;
}

std::vector<double> &DataConfigurations::GetTargetTheta() {
    return this->mTargetTheta;
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

bool DataConfigurations::IsCamelCase(const std::string &aString) {
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

void DataConfigurations::ParseRunMode(const std::string &aRunMode) {
    if (aRunMode == "verbose" || aRunMode == "Verbose") {
        mRunMode = RunMode::VERBOSE_MODE;
    } else if (aRunMode == "standard" || aRunMode == "Standard") {
        mRunMode = RunMode::STANDARD_MODE;
    } else {
        cout << "Error: " << aRunMode << " is not valid " << endl;
        throw range_error("Invalid value. Please use verbose or standard values only.");
    }
}

std::vector<double> DataConfigurations::ParseTheta(const std::string &aInputValues) {
    // Count the number of values in the string
    int num_values = 1;
    for (char aInputValue: aInputValues) {
        if (aInputValue == ':') {
            num_values++;
        }
    }

    // Allocate memory for the array of doubles
    std::vector<double> theta;

    // Split the string into tokens using strtok()
    const char *delim = ":";
    char *token = strtok((char *) aInputValues.c_str(), delim);
//    int i = 1;
    int i = 0;
    while (token != nullptr) {
        // Check if the token is a valid double or "?"
        if (!strcmp(token, "?")) {
            theta.push_back(-1);
        } else {
            try {
                theta.push_back(stod(token));
            }
            catch (...) {
                cout << "Error: " << token << " is not a valid double or '?' " << endl;
                throw range_error("Invalid value. Please use Numerical values only.");
            }
        }

        // Get the next token
        token = strtok(nullptr, delim);
        i++;
    }

    // Check if the number of values in the array is correct
//    if (i != num_values + 1) {
    if (i != num_values) {
        throw range_error(
                "Error: the number of values in the input string is invalid, please use this example format as a reference 1:?:0.1");
    }

    return theta;
}
