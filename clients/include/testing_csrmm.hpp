/* ************************************************************************
 * Copyright 2018 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#pragma once
#ifndef TESTING_CSRMM_HPP
#define TESTING_CSRMM_HPP

#include "rocsparse_test_unique_ptr.hpp"
#include "rocsparse.hpp"
#include "utility.hpp"
#include "unit.hpp"

#include <string>
#include <rocsparse.h>

using namespace rocsparse;
using namespace rocsparse_test;

template <typename T>
void testing_csrmm_bad_arg(void)
{

    rocsparse_int n           = 100;
    rocsparse_int m           = 100;
    rocsparse_int k           = 100;
    rocsparse_int ldb         = 100;
    rocsparse_int ldc         = 100;
    rocsparse_int nnz         = 100;
    rocsparse_int safe_size   = 100;
    T alpha                   = 0.6;
    T beta                    = 0.2;
    rocsparse_operation trans = rocsparse_operation_none;
    rocsparse_status status;

    std::unique_ptr<handle_struct> unique_ptr_handle(new handle_struct);
    rocsparse_handle handle = unique_ptr_handle->handle;

    std::unique_ptr<descr_struct> unique_ptr_descr(new descr_struct);
    rocsparse_mat_descr descr = unique_ptr_descr->descr;

    auto dptr_managed =
        rocsparse_unique_ptr{device_malloc(sizeof(rocsparse_int) * safe_size), device_free};
    auto dcol_managed =
        rocsparse_unique_ptr{device_malloc(sizeof(rocsparse_int) * safe_size), device_free};
    auto dval_managed = rocsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};
    auto dB_managed   = rocsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};
    auto dC_managed   = rocsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};

    rocsparse_int* dptr = (rocsparse_int*)dptr_managed.get();
    rocsparse_int* dcol = (rocsparse_int*)dcol_managed.get();
    T* dval             = (T*)dval_managed.get();
    T* dB               = (T*)dB_managed.get();
    T* dC               = (T*)dC_managed.get();

    if(!dval || !dptr || !dcol || !dB || !dC)
    {
        PRINT_IF_HIP_ERROR(hipErrorOutOfMemory);
        return;
    }

    // testing for(nullptr == dptr)
    {
        rocsparse_int* dptr_null = nullptr;

        status = rocsparse_csrmm(
            handle, trans, m, n, k, nnz, &alpha, descr, dval, dptr_null, dcol, dB, ldb, &beta, dC, ldc);
        verify_rocsparse_status_invalid_pointer(status, "Error: dptr is nullptr");
    }
    // testing for(nullptr == dcol)
    {
        rocsparse_int* dcol_null = nullptr;

        status = rocsparse_csrmm(
            handle, trans, m, n, k, nnz, &alpha, descr, dval, dptr, dcol_null, dB, ldb, &beta, dC, ldc);
        verify_rocsparse_status_invalid_pointer(status, "Error: dcol is nullptr");
    }
    // testing for(nullptr == dval)
    {
        T* dval_null = nullptr;

        status = rocsparse_csrmm(
            handle, trans, m, n, k, nnz, &alpha, descr, dval_null, dptr, dcol, dB, ldb, &beta, dC, ldc);
        verify_rocsparse_status_invalid_pointer(status, "Error: dval is nullptr");
    }
    // testing for(nullptr == dB)
    {
        T* dB_null = nullptr;

        status = rocsparse_csrmm(
            handle, trans, m, n, k, nnz, &alpha, descr, dval, dptr, dcol, dB_null, ldb, &beta, dC, ldc);
        verify_rocsparse_status_invalid_pointer(status, "Error: dB is nullptr");
    }
    // testing for(nullptr == dC)
    {
        T* dC_null = nullptr;

        status = rocsparse_csrmm(
            handle, trans, m, n, k, nnz, &alpha, descr, dval, dptr, dcol, dB, ldb, &beta, dC_null, ldc);
        verify_rocsparse_status_invalid_pointer(status, "Error: dC is nullptr");
    }
    // testing for(nullptr == d_alpha)
    {
        T* d_alpha_null = nullptr;

        status = rocsparse_csrmm(
            handle, trans, m, n, k, nnz, d_alpha_null, descr, dval, dptr, dcol, dB, ldb, &beta, dC, ldc);
        verify_rocsparse_status_invalid_pointer(status, "Error: alpha is nullptr");
    }
    // testing for(nullptr == d_beta)
    {
        T* d_beta_null = nullptr;

        status = rocsparse_csrmm(
            handle, trans, m, n, k, nnz, &alpha, descr, dval, dptr, dcol, dB, ldb, d_beta_null, dC, ldc);
        verify_rocsparse_status_invalid_pointer(status, "Error: beta is nullptr");
    }
    // testing for(nullptr == descr)
    {
        rocsparse_mat_descr descr_null = nullptr;

        status = rocsparse_csrmm(
            handle, trans, m, n, k, nnz, &alpha, descr_null, dval, dptr, dcol, dB, ldb, &beta, dC, ldc);
        verify_rocsparse_status_invalid_pointer(status, "Error: descr is nullptr");
    }
    // testing for(nullptr == handle)
    {
        rocsparse_handle handle_null = nullptr;

        status = rocsparse_csrmm(
            handle_null, trans, m, n, k, nnz, &alpha, descr, dval, dptr, dcol, dB, ldb, &beta, dC, ldc);
        verify_rocsparse_status_invalid_handle(status);
    }
}

template <typename T>
rocsparse_status testing_csrmm(Arguments argus)
{
/*
    rocsparse_int safe_size       = 100;
    rocsparse_int m               = argus.M;
    rocsparse_int n               = argus.N;
    T h_alpha                     = argus.alpha;
    T h_beta                      = argus.beta;
    rocsparse_operation trans     = argus.trans;
    rocsparse_index_base idx_base = argus.idx_base;
    rocsparse_status status;

    std::unique_ptr<handle_struct> test_handle(new handle_struct);
    rocsparse_handle handle = test_handle->handle;

    std::unique_ptr<descr_struct> test_descr(new descr_struct);
    rocsparse_mat_descr descr = test_descr->descr;

    // Set matrix index base
    CHECK_ROCSPARSE_ERROR(rocsparse_set_mat_index_base(descr, idx_base));

    // Determine number of non-zero elements
    double scale = 0.02;
    if(m > 1000 || n > 1000)
    {
        scale = 2.0 / std::max(m, n);
    }
    rocsparse_int nnz = m * scale * n;

    // Argument sanity check before allocating invalid memory
    if(m <= 0 || n <= 0 || nnz <= 0)
    {
        auto dptr_managed =
            rocsparse_unique_ptr{device_malloc(sizeof(rocsparse_int) * safe_size), device_free};
        auto dcol_managed =
            rocsparse_unique_ptr{device_malloc(sizeof(rocsparse_int) * safe_size), device_free};
        auto dval_managed = rocsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};
        auto dx_managed   = rocsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};
        auto dy_managed   = rocsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};

        rocsparse_int* dptr = (rocsparse_int*)dptr_managed.get();
        rocsparse_int* dcol = (rocsparse_int*)dcol_managed.get();
        T* dval             = (T*)dval_managed.get();
        T* dx               = (T*)dx_managed.get();
        T* dy               = (T*)dy_managed.get();

        if(!dval || !dptr || !dcol || !dx || !dy)
        {
            verify_rocsparse_status_success(rocsparse_status_memory_error,
                                            "!dptr || !dcol || !dval || !dx || !dy");
            return rocsparse_status_memory_error;
        }

        CHECK_ROCSPARSE_ERROR(rocsparse_set_pointer_mode(handle, rocsparse_pointer_mode_host));
        status = rocsparse_csrmm(
            handle, trans, m, n, nnz, &h_alpha, descr, dval, dptr, dcol, dx, &h_beta, dy);

        if(m < 0 || n < 0 || nnz < 0)
        {
            verify_rocsparse_status_invalid_size(status, "Error: m < 0 || n < 0 || nnz < 0");
        }
        else
        {
            verify_rocsparse_status_success(status, "m >= 0 && n >= 0 && nnz >= 0");
        }

        return rocsparse_status_success;
    }

    // Host structures
    std::vector<rocsparse_int> hcsr_row_ptr;
    std::vector<rocsparse_int> hcoo_row_ind;
    std::vector<rocsparse_int> hcol_ind;
    std::vector<T> hval;

    // Initial Data on CPU
    srand(12345ULL);
    if(argus.laplacian)
    {
        m = n = gen_2d_laplacian(argus.laplacian, hcsr_row_ptr, hcol_ind, hval, idx_base);
        nnz   = hcsr_row_ptr[m];
    }
    else
    {
        if(argus.filename != "")
        {
            if(read_mtx_matrix(argus.filename.c_str(), m, n, nnz, hcoo_row_ind, hcol_ind, hval) !=
               0)
            {
                fprintf(stderr, "Cannot open [read] %s\n", argus.filename.c_str());
                return rocsparse_status_internal_error;
            }
        }
        else
        {
            gen_matrix_coo(m, n, nnz, hcoo_row_ind, hcol_ind, hval, idx_base);
        }

        // Convert COO to CSR
        if(!argus.laplacian)
        {
            hcsr_row_ptr.resize(m + 1, 0);
            for(rocsparse_int i = 0; i < nnz; ++i)
            {
                ++hcsr_row_ptr[hcoo_row_ind[i] + 1 - idx_base];
            }

            hcsr_row_ptr[0] = idx_base;
            for(rocsparse_int i = 0; i < m; ++i)
            {
                hcsr_row_ptr[i + 1] += hcsr_row_ptr[i];
            }
        }
    }

    std::vector<T> hx(n);
    std::vector<T> hy_1(m);
    std::vector<T> hy_2(m);
    std::vector<T> hy_gold(m);

    rocsparse_init<T>(hx, 1, n);
    rocsparse_init<T>(hy_1, 1, m);

    // copy vector is easy in STL; hy_gold = hx: save a copy in hy_gold which will be output of CPU
    hy_2    = hy_1;
    hy_gold = hy_1;

    // allocate memory on device
    auto dptr_managed =
        rocsparse_unique_ptr{device_malloc(sizeof(rocsparse_int) * (m + 1)), device_free};
    auto dcol_managed =
        rocsparse_unique_ptr{device_malloc(sizeof(rocsparse_int) * nnz), device_free};
    auto dval_managed    = rocsparse_unique_ptr{device_malloc(sizeof(T) * nnz), device_free};
    auto dx_managed      = rocsparse_unique_ptr{device_malloc(sizeof(T) * n), device_free};
    auto dy_1_managed    = rocsparse_unique_ptr{device_malloc(sizeof(T) * m), device_free};
    auto dy_2_managed    = rocsparse_unique_ptr{device_malloc(sizeof(T) * m), device_free};
    auto d_alpha_managed = rocsparse_unique_ptr{device_malloc(sizeof(T)), device_free};
    auto d_beta_managed  = rocsparse_unique_ptr{device_malloc(sizeof(T)), device_free};

    rocsparse_int* dptr = (rocsparse_int*)dptr_managed.get();
    rocsparse_int* dcol = (rocsparse_int*)dcol_managed.get();
    T* dval             = (T*)dval_managed.get();
    T* dx               = (T*)dx_managed.get();
    T* dy_1             = (T*)dy_1_managed.get();
    T* dy_2             = (T*)dy_2_managed.get();
    T* d_alpha          = (T*)d_alpha_managed.get();
    T* d_beta           = (T*)d_beta_managed.get();

    if(!dval || !dptr || !dcol || !dx || !dy_1 || !dy_2 || !d_alpha || !d_beta)
    {
        verify_rocsparse_status_success(rocsparse_status_memory_error,
                                        "!dval || !dptr || !dcol || !dx || "
                                        "!dy_1 || !dy_2 || !d_alpha || !d_beta");
        return rocsparse_status_memory_error;
    }

    // copy data from CPU to device
    CHECK_HIP_ERROR(hipMemcpy(
        dptr, hcsr_row_ptr.data(), sizeof(rocsparse_int) * (m + 1), hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(
        hipMemcpy(dcol, hcol_ind.data(), sizeof(rocsparse_int) * nnz, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dval, hval.data(), sizeof(T) * nnz, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dx, hx.data(), sizeof(T) * n, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(dy_1, hy_1.data(), sizeof(T) * m, hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(d_alpha, &h_alpha, sizeof(T), hipMemcpyHostToDevice));
    CHECK_HIP_ERROR(hipMemcpy(d_beta, &h_beta, sizeof(T), hipMemcpyHostToDevice));

    if(argus.unit_check)
    {
        CHECK_HIP_ERROR(hipMemcpy(dy_2, hy_2.data(), sizeof(T) * m, hipMemcpyHostToDevice));

        // ROCSPARSE pointer mode host
        CHECK_ROCSPARSE_ERROR(rocsparse_set_pointer_mode(handle, rocsparse_pointer_mode_host));
        CHECK_ROCSPARSE_ERROR(rocsparse_csrmm(
            handle, trans, m, n, nnz, &h_alpha, descr, dval, dptr, dcol, dx, &h_beta, dy_1));

        // ROCSPARSE pointer mode device
        CHECK_ROCSPARSE_ERROR(rocsparse_set_pointer_mode(handle, rocsparse_pointer_mode_device));
        CHECK_ROCSPARSE_ERROR(rocsparse_csrmm(
            handle, trans, m, n, nnz, d_alpha, descr, dval, dptr, dcol, dx, d_beta, dy_2));

        // copy output from device to CPU
        CHECK_HIP_ERROR(hipMemcpy(hy_1.data(), dy_1, sizeof(T) * m, hipMemcpyDeviceToHost));
        CHECK_HIP_ERROR(hipMemcpy(hy_2.data(), dy_2, sizeof(T) * m, hipMemcpyDeviceToHost));

        // CPU
        double cpu_time_used = get_time_us();

        for(rocsparse_int i = 0; i < m; ++i)
        {
            hy_gold[i] *= h_beta;
            for(rocsparse_int j = hcsr_row_ptr[i] - idx_base; j < hcsr_row_ptr[i + 1] - idx_base;
                ++j)
            {
                hy_gold[i] += h_alpha * hval[j] * hx[hcol_ind[j] - idx_base];
            }
        }

        cpu_time_used = get_time_us() - cpu_time_used;

        // enable unit check, notice unit check is not invasive, but norm check is,
        // unit check and norm check can not be interchanged their order
        if(argus.unit_check)
        {
            unit_check_general(1, m, hy_gold.data(), hy_1.data());
            unit_check_general(1, m, hy_gold.data(), hy_2.data());
        }
    }

    if(argus.timing)
    {
        int number_cold_calls = 2;
        int number_hot_calls  = argus.iters;
        CHECK_ROCSPARSE_ERROR(rocsparse_set_pointer_mode(handle, rocsparse_pointer_mode_host));

        for(int iter = 0; iter < number_cold_calls; iter++)
        {
            rocsparse_csrmm(
                handle, trans, m, n, nnz, &h_alpha, descr, dval, dptr, dcol, dx, &h_beta, dy_1);
        }

        double gpu_time_used = get_time_us(); // in microseconds

        for(int iter = 0; iter < number_hot_calls; iter++)
        {
            rocsparse_csrmm(
                handle, trans, m, n, nnz, &h_alpha, descr, dval, dptr, dcol, dx, &h_beta, dy_1);
        }

        // Convert to miliseconds per call
        gpu_time_used     = (get_time_us() - gpu_time_used) / (number_hot_calls * 1e3);
        size_t flops      = (h_alpha != 1.0) ? 3.0 * nnz : 2.0 * nnz;
        flops             = (h_beta != 0.0) ? flops + m : flops;
        double gpu_gflops = flops / gpu_time_used / 1e6;
        size_t memtrans   = 2.0 * m + nnz;
        memtrans          = (h_beta != 0.0) ? memtrans + m : memtrans;
        double bandwidth =
            (memtrans * sizeof(T) + (m + 1 + nnz) * sizeof(rocsparse_int)) / gpu_time_used / 1e6;

        printf("m\t\tn\t\tnnz\t\talpha\tbeta\tGFlops\tGB/s\tmsec\n");
        printf("%8d\t%8d\t%9d\t%0.2lf\t%0.2lf\t%0.2lf\t%0.2lf\t%0.2lf\n",
               m,
               n,
               nnz,
               h_alpha,
               h_beta,
               gpu_gflops,
               bandwidth,
               gpu_time_used);
    }
*/    
    return rocsparse_status_success;
}

#endif // TESTING_CSRMM_HPP
