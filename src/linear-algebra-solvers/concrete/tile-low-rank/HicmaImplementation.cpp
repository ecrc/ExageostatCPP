
// Copyright (c) 2017-2023 King Abdullah University of Science and Technology,
// Copyright (C) 2023 by Brightskies inc,
// All rights reserved.
// ExaGeoStat is a software package, provided by King Abdullah University of Science and Technology (KAUST).

/**
 * @file HicmaImplementation.cpp
 * @brief Sets up the HiCMA descriptors needed for the tile low rank computations in ExaGeoStat.
 * @version 1.0.0
 * @author Sameh Abdulah
 * @date 2023-03-26
**/

#include <linear-algebra-solvers/concrete/tile-low-rank/HicmaImplementation.hpp>
#include <lapacke.h>

extern "C" {
#include <hicma.h>
#include <control/hicma_context.h>
}

using namespace exageostat::linearAlgebra::tileLowRank;
using namespace exageostat::common;
using namespace exageostat::dataunits;
using namespace exageostat::kernels;
using namespace std;

template<typename T>
void HicmaImplementation<T>::InitiateDescriptors() {

    // Check for Initialise the Hicma context.
    if (!this->apContext) {
        throw std::runtime_error(
                "ExaGeoStat hardware is not initialized, please use 'ExaGeoStat<double/float>::ExaGeoStatInitializeHardware(configurations)'.");
    }
    vector<void *> &pDescriptorC = this->mpConfigurations->GetDescriptorC();
    vector<void *> &pDescriptorZ = this->mpConfigurations->GetDescriptorZ();
    auto pDescriptorZcpy = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorZcpy();
    auto pDescriptorDeterminant = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorDeterminant();

    // Create a Hicma sequence
    HICMA_sequence_t *pSequence;
    HICMA_request_t request[2] = {HICMA_SUCCESS, HICMA_SUCCESS};
    HICMA_Sequence_Create(&pSequence);

    int N = this->mpConfigurations->GetProblemSize();
    int lts = this->mpConfigurations->GetLowTileSize();
    int pGrid = this->mpConfigurations->GetPGrid();
    int qGrid = this->mpConfigurations->GetQGrid();
    bool isOOC = this->mpConfigurations->GetIsOOC();
    int maxRank = this->mpConfigurations->GetMaxRank();
    int nZmiss = this->mpConfigurations->GetUnknownObservationsNb();
    double meanSquareError = this->mpConfigurations->GetMeanSquareError();
    int approximationMode = this->mpConfigurations->GetApproximationMode();
    string actualObservationsFilePath = this->mpConfigurations->GetActualObservationsFilePath();
    double determinantValue = this->mpConfigurations->GetDeterminantValue();

    int nZobsValue;
    if (actualObservationsFilePath.empty()) {
        nZobsValue = N - nZmiss;
    } else {
        nZobsValue = N;
    }

    this->mpConfigurations->SetKnownObservationsValues(nZobsValue);
    int nZobs = this->mpConfigurations->GetKnownObservationsValues();

    // For distributed system and should be removed
    T *Zcpy = (T *) malloc(N * sizeof(T));

    pDescriptorC.push_back(nullptr);
    auto **pHicmaDescriptorC = (HICMA_desc_t **) &pDescriptorC[0];

    pDescriptorZ.push_back(nullptr);
    auto **pHicmaDescriptorZ = (HICMA_desc_t **) &pDescriptorZ[0];

    auto **pDescriptorCD = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorCD()[0];
    auto **pDescriptorC12D = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorCD()[1];
    auto **pDescriptorC22D = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorCD()[2];

    auto **pDescriptorCUV = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorCUV()[0];
    auto **pDescriptorC12UV = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorCUV()[1];
    auto **pDescriptorC22UV = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorCUV()[2];

    auto **pDescriptorCrk = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorCrk()[0];
    auto **pDescriptorC12rk = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorCrk()[1];
    auto **pDescriptorC22rk = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorCrk()[2];

    auto **pDescriptorZObservations = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorZObservations();
    auto **pDescriptorZactual = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorZActual();
    auto **pDescriptorMSE = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorMSE();

    int MBC, NBC, MC, NC;
    int MBD, NBD, MD, ND;
    int MBUV, NBUV, MUV, NUV;
    int MBrk, NBrk, Mrk, Nrk;

    FloatPoint floatPoint;
    if (sizeof(T) == SIZE_OF_FLOAT) {
        floatPoint = EXAGEOSTAT_REAL_FLOAT;
    } else {
        floatPoint = EXAGEOSTAT_REAL_DOUBLE;
    }

    //CDense Descriptor
    if (approximationMode == 1) {
        MBC = lts;
        NBC = lts;
        MC = N;
        NC = N;
    } else {
        MBC = 1;
        NBC = 1;
        MC = lts;
        NC = lts;
    }

    EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pHicmaDescriptorC, isOOC, nullptr, (HICMA_enum) floatPoint, MBC, NBC,
                                           MBC * NBC, MC, NC, 0, 0, MC, NC, pGrid, qGrid)

    //CAD Descriptor
    MBD = lts;
    NBD = lts;
    MD = N;
    ND = MBD;
    EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorCD, isOOC, nullptr, (HICMA_enum) floatPoint, MBD, NBD, MBD * NBD,
                                           MD, ND, 0, 0, MD, ND, pGrid, qGrid)

    //CUV Descriptor
    MBUV = lts;
    NBUV = 2 * maxRank;
    int N_over_lts_times_lts = N / lts * lts;
    if (N_over_lts_times_lts < N) {
        MUV = N_over_lts_times_lts + lts;
    } else if (N_over_lts_times_lts == N) {
        MUV = N_over_lts_times_lts;
    } else {
        throw range_error("Invalid value. This case should not happen, Please make sure of N and lts values.");
    }

    T expr = (T) MUV / (T) lts;
    NUV = 2 * expr * maxRank;
    EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorCUV, isOOC, nullptr, (HICMA_enum) floatPoint, MBUV, NBUV,
                                           MBUV * NBUV, MUV, NUV, 0, 0, MUV, NUV, pGrid, qGrid)

    //Crk Descriptor
    MBrk = 1;
    NBrk = 1;
    Mrk = (*pDescriptorCUV)->mt;
    Nrk = (*pDescriptorCUV)->mt;
    EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorCrk, isOOC, nullptr, (HICMA_enum) floatPoint, MBrk, NBrk,
                                           MBrk * NBrk, Mrk, Nrk, 0, 0, Mrk, Nrk, pGrid, qGrid)

    HICMA_Sequence_Create(&pSequence);
    EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pHicmaDescriptorZ, isOOC, nullptr, (HICMA_enum) floatPoint, lts, lts,
                                           lts * lts, N, 1, 0, 0, N, 1, pGrid, qGrid)

    EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorZcpy, isOOC, Zcpy, (HICMA_enum) floatPoint, lts, lts, lts * lts,
                                           N, 1, 0, 0, N, 1, pGrid, qGrid)
    EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorDeterminant, isOOC, &determinantValue, (HICMA_enum) floatPoint,
                                           lts, lts, lts * lts, 1, 1, 0, 0, 1, 1, pGrid, qGrid)

    if (nZmiss != 0) {
        if (actualObservationsFilePath.empty()) {
            EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorZObservations, isOOC, &pDescriptorZcpy[nZmiss],
                                                   (HICMA_enum) floatPoint, lts, lts, lts * lts, nZobs, 1, 0, 0, nZobs,
                                                   1, pGrid, qGrid)
            EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorZactual, isOOC, pDescriptorZcpy, (HICMA_enum) floatPoint,
                                                   lts, lts, lts * lts, nZmiss, 1, 0, 0, nZmiss, 1, pGrid, qGrid)
        } else {

            EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorZObservations, isOOC, nullptr, (HICMA_enum) floatPoint,
                                                   lts,
                                                   lts, lts * lts, nZmiss, 1, 0, 0, nZmiss, 1, pGrid, qGrid)
            EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorZactual, isOOC, nullptr, (HICMA_enum) floatPoint, lts,
                                                   lts, lts * lts, nZmiss, 1, 0, 0, nZmiss, 1, pGrid, qGrid)
        }
        //C12AD Descriptor
        MBD = lts;
        NBD = lts;
        MD = nZmiss;
        ND = MBD;
        EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorC12D, isOOC, nullptr, (HICMA_enum) floatPoint, MBD, NBD,
                                               MBD * NBD, MD, ND, 0, 0, MD, ND, pGrid, qGrid)

        //C12UV Descriptor
        MBUV = lts;
        NBUV = 2 * maxRank;
        EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorC12UV, isOOC, nullptr, (HICMA_enum) floatPoint, MBUV, NBUV,
                                               MBUV * NBUV, MBUV, NBUV, 0, 0, MBUV, NBUV, pGrid, qGrid)

        //C12Ark Descriptor
        MBrk = 1;
        NBrk = 1;
        Mrk = (*pDescriptorC12UV)->mt;
        Nrk = (*pDescriptorC12UV)->mt;
        EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorC12rk, isOOC, nullptr, (HICMA_enum) floatPoint, MBrk, NBrk,
                                               MBrk * NBrk, Mrk, Nrk, 0, 0, Mrk, Nrk, pGrid, qGrid)

        //C22D Descriptor
        MBD = lts;
        NBD = lts;
        MD = nZobs;
        ND = MBD;
        EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorC22D, isOOC, nullptr, (HICMA_enum) floatPoint, MBD, NBD,
                                               MBD * NBD, MD, ND, 0, 0, MD, ND, pGrid, qGrid)

        //C22UV Descriptor
        MBUV = lts;
        NBUV = 2 * maxRank;
        EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorC22UV, isOOC, nullptr, (HICMA_enum) floatPoint, MBUV, NBUV,
                                               MBUV * NBUV, MBUV, NBUV, 0, 0, MBUV, NBUV, pGrid, qGrid)

        //C22Ark Descriptor
        MBrk = 1;
        NBrk = 1;
        Mrk = (*pDescriptorC22UV)->mt;
        Nrk = (*pDescriptorC22UV)->mt;
        EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorC22rk, isOOC, nullptr, (HICMA_enum) floatPoint, MBrk, NBrk,
                                               MBrk * NBrk, Mrk, Nrk, 0, 0, Mrk, Nrk, pGrid, qGrid)

        //Other descriptors
        EXAGEOSTAT_ALLOCATE_APPROX_MATRIX_TILE(pDescriptorMSE, isOOC, &meanSquareError, (HICMA_enum) floatPoint, lts,
                                               lts, lts * lts, 1, 1, 0, 0, 1, 1, pGrid, qGrid)
    }

    this->mpConfigurations->SetSequence(pSequence);
    this->mpConfigurations->SetRequest(request);

    //stop gsl error handler
    gsl_set_error_handler_off();
}

template<typename T>
void HicmaImplementation<T>::ExaGeoStatInitContext(const int &apCoresNumber, const int &apGPUs) {

    if (!this->apContext) {
        HICMA_user_tag_size(31, 26);
        HICMA_Init(apCoresNumber, apGPUs);
        this->apContext = hicma_context_self();
    }
}

template<typename T>
void HicmaImplementation<T>::ExaGeoStatFinalizeContext() {

    if (!this->apContext) {
        cout
                << "No initialised context of HiCMA, Please use 'ExaGeoStat<double/or/float>::ExaGeoStatInitializeHardware(configurations);'"
                << endl;
    } else {
        HICMA_Finalize();
        this->apContext = nullptr;
    }
}

#define starpu_mpi_codelet(_codelet_) _codelet_

static void cl_dcmg_cpu_func(void *buffers[], void *cl_arg) {

    int m, n, m0, n0;
    exageostat::dataunits::Locations *apLocation1;
    exageostat::dataunits::Locations *apLocation2;
    exageostat::dataunits::Locations *apLocation3;
    double *theta;
    double *A;
    int distance_metric;
    exageostat::kernels::Kernel *kernel;

    A = (double *) STARPU_MATRIX_GET_PTR(buffers[0]);

    starpu_codelet_unpack_args(cl_arg, &m, &n, &m0, &n0, &apLocation1, &apLocation2, &apLocation3, &theta,
                               &distance_metric, &kernel);
    kernel->GenerateCovarianceMatrix(A, m, n, m0, n0, apLocation1,
                                     apLocation2, apLocation3, theta, distance_metric);
}


static struct starpu_codelet cl_dcmg =
        {
                .where        = STARPU_CPU /*| STARPU_CUDA*/,
                .cpu_func     = cl_dcmg_cpu_func,
#if defined(EXAGEOSTAT_USE_CUDA)
                //    .cuda_func      = {cl_dcmg_cuda_func},
#endif
                .nbuffers     = 1,
                .modes        = {STARPU_W},
                .name         = "dcmg"
        };

template<typename T>
void
HicmaImplementation<T>::CovarianceMatrixCodelet(void *descA, int uplo, dataunits::Locations *apLocation1,
                                                dataunits::Locations *apLocation2,
                                                dataunits::Locations *apLocation3, double *apLocalTheta,
                                                int aDistanceMetric,
                                                exageostat::kernels::Kernel *apKernel) {

    // Check for Initialise the Hicma context.
    if (!this->apContext) {
        throw std::runtime_error(
                "ExaGeoStat hardware is not initialized, please use 'ExaGeoStat<double/float>::ExaGeoStatInitializeHardware(configurations)'.");
    }

    HICMA_option_t options;
    HICMA_RUNTIME_options_init(&options, (HICMA_context_t *) this->apContext,
                               (HICMA_sequence_t *) this->mpConfigurations->GetSequence(),
                               (HICMA_request_t *) this->mpConfigurations->GetRequest());

    int tempmm, tempnn;

    auto *HICMA_descA = (HICMA_desc_t *) descA;
    HICMA_desc_t A = *HICMA_descA;
    struct starpu_codelet *cl = &cl_dcmg;
    int m, n, m0, n0;

    for (n = 0; n < A.nt; n++) {
        tempnn = n == A.nt - 1 ? A.n - n * A.nb : A.nb;
        if (uplo == HicmaUpperLower) {
            m = 0;
        } else {
            m = A.m == A.n ? n : 0;
        }
        for (; m < A.mt; m++) {

            tempmm = m == A.mt - 1 ? A.m - m * A.mb : A.mb;
            m0 = m * A.mb;
            n0 = n * A.nb;

            // Register the data with StarPU
            starpu_insert_task(starpu_mpi_codelet(cl),
                               STARPU_VALUE, &tempmm, sizeof(int),
                               STARPU_VALUE, &tempnn, sizeof(int),
                               STARPU_VALUE, &m0, sizeof(int),
                               STARPU_VALUE, &n0, sizeof(int),
                               STARPU_W, (starpu_data_handle_t) HICMA_RUNTIME_data_getaddr(HICMA_descA, m, n),
                               STARPU_VALUE, &apLocation1, sizeof(dataunits::Locations *),
                               STARPU_VALUE, &apLocation2, sizeof(dataunits::Locations *),
                               STARPU_VALUE, &apLocation3, sizeof(dataunits::Locations *),
                               STARPU_VALUE, &apLocalTheta, sizeof(double *),
                               STARPU_VALUE, &aDistanceMetric, sizeof(int),
                               STARPU_VALUE, &apKernel, sizeof(exageostat::kernels::Kernel *),
                               0);

            auto handle = (starpu_data_handle_t) HICMA_RUNTIME_data_getaddr(HICMA_descA, m, n);
            this->apMatrix = (double *) starpu_variable_get_local_ptr(handle);
        }
    }
    HICMA_RUNTIME_options_ws_free(&options);
    HICMA_RUNTIME_options_finalize(&options, (HICMA_context_t *) this->apContext);

    HICMA_Sequence_Wait((HICMA_sequence_t *) this->mpConfigurations->GetSequence());

    // Unregister Handles
    for (n = 0; n < A.nt; n++) {
        tempnn = n == A.nt - 1 ? A.n - n * A.nb : A.nb;
        if (uplo == HicmaUpperLower) {
            m = 0;
        } else {
            m = A.m == A.n ? n : 0;
        }
        for (; m < A.mt; m++) {
            starpu_data_unregister((starpu_data_handle_t) HICMA_RUNTIME_data_getaddr(HICMA_descA, m, n));
        }
    }
}

template<typename T>
void HicmaImplementation<T>::GenerateObservationsVector(void *descA, Locations *apLocation1,
                                                        Locations *apLocation2, Locations *apLocation3,
                                                        vector<double> aLocalTheta, int aDistanceMetric,
                                                        Kernel *apKernel) {

    // Check for Initialise the Hicma context.
    if (!this->apContext) {
        throw std::runtime_error(
                "ExaGeoStat hardware is not initialized, please use 'ExaGeoStat<double/float>::ExaGeoStatInitializeHardware(configurations)'.");
    }

    auto *sequence = (HICMA_sequence_t *) this->mpConfigurations->GetSequence();
    auto *request = (HICMA_request_t *) this->mpConfigurations->GetRequest();
    int N = this->mpConfigurations->GetProblemSize();

    //// TODO: Make all zeros, Seed.
    int iseed[4] = {0, 0, 0, 1};
    //nomral random generation of e -- ei~N(0, 1) to generate Z
    auto *Nrand = (double *) malloc(N * sizeof(double));
    LAPACKE_dlarnv(3, iseed, N, Nrand);

    //Generate the co-variance matrix C
//    VERBOSE("Initializing Covariance Matrix (Synthetic Dataset Generation Phase).....");
    auto *theta = (double *) malloc(aLocalTheta.size() * sizeof(double));
    for (int i = 0; i < aLocalTheta.size(); i++) {
        theta[i] = aLocalTheta[i];
    }
    this->CovarianceMatrixCodelet(descA, EXAGEOSTAT_LOWER, apLocation1, apLocation2, apLocation3, theta,
                                  aDistanceMetric, apKernel);
    free(theta);
    free(Nrand);
}
template<typename T>
void HicmaImplementation<T>::DestoryDescriptors() {

    vector<void *> &pDescriptorC = this->mpConfigurations->GetDescriptorC();
    vector<void *> &pDescriptorZ = this->mpConfigurations->GetDescriptorZ();
    auto pHicmaDescriptorZcpy = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorZcpy();
    vector<void *> &pDescriptorProduct = this->mpConfigurations->GetDescriptorProduct();
    auto pHicmaDescriptorDeterminant = (HICMA_desc_t **) &this->mpConfigurations->GetDescriptorDeterminant();

    for(auto & descC : pDescriptorC){
        if(!descC){
            HICMA_Desc_Destroy((HICMA_desc_t **) &descC);
        }
    }
    for(auto & descZ : pDescriptorZ){
        if(!descZ){
            HICMA_Desc_Destroy((HICMA_desc_t **) &descZ);
        }
    }
    for(auto & descProduct : pDescriptorProduct){
        if(!descProduct){
            HICMA_Desc_Destroy((HICMA_desc_t **) &descProduct);
        }
    }
    if(!pHicmaDescriptorZcpy){
        HICMA_Desc_Destroy(pHicmaDescriptorZcpy);
    }
    if(!pHicmaDescriptorDeterminant){
        HICMA_Desc_Destroy(pHicmaDescriptorDeterminant);
    }

    if(!(HICMA_sequence_t *) this->mpConfigurations->GetSequence()){
        HICMA_Sequence_Destroy((HICMA_sequence_t *) this->mpConfigurations->GetSequence());
    }
}
namespace exageostat::linearAlgebra::tileLowRank {
    template<typename T> void *HicmaImplementation<T>::apContext = nullptr;
}