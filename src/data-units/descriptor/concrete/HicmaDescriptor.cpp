/**
 * @file HicmaDescriptor.cpp
 * @brief Defines the Hicma Descriptor class for creating matrix descriptors using the HICMA library.
 * @version 1.0.0
 * @author Sameh Abdulah
 * @author Mahmoud ElKarargy
 * @date 2023-08-15
**/
#include <data-units/descriptor/concrete/HicmaDescriptor.hpp>

using namespace exageostat::dataunits::descriptor;

template<typename T>
HICMA_desc_t *HicmaDescriptor<T>::CreateHicmaDescriptor(void *apDescriptor, const bool &aIsOOC, void *apMatrix,
                                                        const common::FloatPoint &aFloatPoint, const int &aMB,
                                                        const int &aNB, const int &aSize, const int &aLM,
                                                        const int &aLN, const int &aI, const int &aJ,
                                                        const int &aM, const int &aN, const int &aP,
                                                        const int &aQ) {
    auto hicma_desc = (HICMA_desc_t *) apDescriptor;
    if (aIsOOC && apMatrix == nullptr && aMB != 1 && aNB != 1) {
        HICMA_Desc_Create_OOC(&hicma_desc, (HICMA_enum) aFloatPoint, aMB, aNB, aSize, aLM, aLN, aI, aJ, aM, aN, aP, aQ);
    } else {
        HICMA_Desc_Create(&hicma_desc, apMatrix, (HICMA_enum) aFloatPoint, aMB, aNB, aSize, aLM, aLN, aI, aJ, aM, aN,
                          aP, aQ);
    }
    return hicma_desc;
}

template<typename T>
int HicmaDescriptor<T>::DestroyHicmaDescriptor(void *apDesc) {
    auto hicma_desc = (HICMA_desc_t *) apDesc;
    return HICMA_Desc_Destroy(&hicma_desc);
}
