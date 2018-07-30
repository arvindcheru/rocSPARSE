/* ************************************************************************
 * Copyright 2018 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#pragma once
#ifndef TESTING_HYBMV_HPP
#define TESTING_HYBMV_HPP

#include "rocsparse_test_unique_ptr.hpp"
#include "rocsparse.hpp"
#include "utility.hpp"
#include "unit.hpp"

#include <string>
#include <rocsparse.h>

using namespace rocsparse;
using namespace rocsparse_test;

struct testhyb
{
    rocsparse_int m;
    rocsparse_int n;
    rocsparse_hyb_partition partition;
    rocsparse_int ell_nnz;
    rocsparse_int ell_width;
    rocsparse_int* ell_col_ind;
    void* ell_val;
    rocsparse_int coo_nnz;
    rocsparse_int* coo_row_ind;
    rocsparse_int* coo_col_ind;
    void* coo_val;
};

template <typename T>
void testing_hybmv_bad_arg(void)
{
    rocsparse_int safe_size   = 100;
    T alpha                   = 0.6;
    T beta                    = 0.2;
    rocsparse_operation transA = rocsparse_operation_none;
    rocsparse_status status;

    std::unique_ptr<handle_struct> unique_ptr_handle(new handle_struct);
    rocsparse_handle handle = unique_ptr_handle->handle;

    std::unique_ptr<descr_struct> unique_ptr_descr(new descr_struct);
    rocsparse_mat_descr descr = unique_ptr_descr->descr;

    std::unique_ptr<hyb_struct> unique_ptr_hyb(new hyb_struct);
    rocsparse_hyb_mat hyb = unique_ptr_hyb->hyb;

    auto dx_managed = rocsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};
    auto dy_managed = rocsparse_unique_ptr{device_malloc(sizeof(T) * safe_size), device_free};

    T* dx = (T*)dx_managed.get();
    T* dy = (T*)dy_managed.get();

    if(!dx || !dy)
    {
        PRINT_IF_HIP_ERROR(hipErrorOutOfMemory);
        return;
    }

    // testing for(nullptr == dx)
    {
        T* dx_null = nullptr;

        status = rocsparse_hybmv(handle, transA, &alpha, descr, hyb, dx_null, &beta, dy);
        verify_rocsparse_status_invalid_pointer(status, "Error: dx is nullptr");
    }
    // testing for(nullptr == dy)
    {
        T* dy_null = nullptr;

        status = rocsparse_hybmv(handle, transA, &alpha, descr, hyb, dx, &beta, dy_null);
        verify_rocsparse_status_invalid_pointer(status, "Error: dy is nullptr");
    }
    // testing for(nullptr == d_alpha)
    {
        T* d_alpha_null = nullptr;

        status = rocsparse_hybmv(handle, transA, d_alpha_null, descr, hyb, dx, &beta, dy);
        verify_rocsparse_status_invalid_pointer(status, "Error: alpha is nullptr");
    }
    // testing for(nullptr == d_beta)
    {
        T* d_beta_null = nullptr;

        status = rocsparse_hybmv(handle, transA, &alpha, descr, hyb, dx, d_beta_null, dy);
        verify_rocsparse_status_invalid_pointer(status, "Error: beta is nullptr");
    }
    // testing for(nullptr == hyb)
    {
        rocsparse_hyb_mat hyb_null = nullptr;

        status = rocsparse_hybmv(handle, transA, &alpha, descr, hyb_null, dx, &beta, dy);
        verify_rocsparse_status_invalid_pointer(status, "Error: descr is nullptr");
    }
    // testing for(nullptr == descr)
    {
        rocsparse_mat_descr descr_null = nullptr;

        status = rocsparse_hybmv(handle, transA, &alpha, descr_null, hyb, dx, &beta, dy);
        verify_rocsparse_status_invalid_pointer(status, "Error: descr is nullptr");
    }
    // testing for(nullptr == handle)
    {
        rocsparse_handle handle_null = nullptr;

        status = rocsparse_hybmv(handle_null, transA, &alpha, descr, hyb, dx, &beta, dy);
        verify_rocsparse_status_invalid_handle(status);
    }
}

template <typename T>
rocsparse_status testing_hybmv(Arguments argus)
{
    rocsparse_int safe_size       = 100;
    rocsparse_int m               = argus.M;
    rocsparse_int n               = argus.N;
    T h_alpha                     = argus.alpha;
    T h_beta                      = argus.beta;
    rocsparse_operation transA     = argus.transA;
    rocsparse_index_base idx_base = argus.idx_base;
    rocsparse_hyb_partition part  = argus.part;
    rocsparse_int user_ell_width  = argus.ell_width;
    rocsparse_status status;

    std::unique_ptr<handle_struct> test_handle(new handle_struct);
    rocsparse_handle handle = test_handle->handle;

    std::unique_ptr<descr_struct> test_descr(new descr_struct);
    rocsparse_mat_descr descr = test_descr->descr;

    // Set matrix index base
    CHECK_ROCSPARSE_ERROR(rocsparse_set_mat_index_base(descr, idx_base));

    std::unique_ptr<hyb_struct> test_hyb(new hyb_struct);
    rocsparse_hyb_mat hyb = test_hyb->hyb;

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
        status =
            rocsparse_csr2hyb(handle, m, n, descr, dval, dptr, dcol, hyb, user_ell_width, part);

        if(m < 0 || n < 0 || nnz < 0)
        {
            verify_rocsparse_status_invalid_size(status, "Error: m < 0 || n < 0 || nnz < 0");
        }

        // hybmv should be able to deal with m <= 0 || n <= 0 || nnz <= 0 even if csr2hyb fails
        // because hyb structures is allocated with n = m = 0 - so nothing should happen
        status = rocsparse_hybmv(handle, transA, &h_alpha, descr, hyb, dx, &h_beta, dy);
        verify_rocsparse_status_success(status, "m >= 0 && n >= 0 && nnz >= 0");

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

    // User given ELL width
    if(part == rocsparse_hyb_partition_user)
    {
        user_ell_width = user_ell_width * nnz / m;
    }

    // Convert CSR to HYB
    CHECK_ROCSPARSE_ERROR(
        rocsparse_csr2hyb(handle, m, n, descr, dval, dptr, dcol, hyb, user_ell_width, part));

    if(argus.unit_check)
    {
        CHECK_HIP_ERROR(hipMemcpy(dy_2, hy_2.data(), sizeof(T) * m, hipMemcpyHostToDevice));

        // ROCSPARSE pointer mode host
        CHECK_ROCSPARSE_ERROR(rocsparse_set_pointer_mode(handle, rocsparse_pointer_mode_host));
        CHECK_ROCSPARSE_ERROR(
            rocsparse_hybmv(handle, transA, &h_alpha, descr, hyb, dx, &h_beta, dy_1));

        // ROCSPARSE pointer mode device
        CHECK_ROCSPARSE_ERROR(rocsparse_set_pointer_mode(handle, rocsparse_pointer_mode_device));
        CHECK_ROCSPARSE_ERROR(
            rocsparse_hybmv(handle, transA, d_alpha, descr, hyb, dx, d_beta, dy_2));

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

        unit_check_general(1, m, hy_gold.data(), hy_1.data());
        unit_check_general(1, m, hy_gold.data(), hy_2.data());
    }

    if(argus.timing)
    {
        int number_cold_calls = 2;
        int number_hot_calls  = argus.iters;
        CHECK_ROCSPARSE_ERROR(rocsparse_set_pointer_mode(handle, rocsparse_pointer_mode_host));

        for(int iter = 0; iter < number_cold_calls; iter++)
        {
            rocsparse_hybmv(handle, transA, &h_alpha, descr, hyb, dx, &h_beta, dy_1);
        }

        double gpu_time_used = get_time_us(); // in microseconds

        for(int iter = 0; iter < number_hot_calls; iter++)
        {
            rocsparse_hybmv(handle, transA, &h_alpha, descr, hyb, dx, &h_beta, dy_1);
        }

        testhyb* dhyb = (testhyb*)hyb;

        // Convert to miliseconds per call
        gpu_time_used     = (get_time_us() - gpu_time_used) / (number_hot_calls * 1e3);
        size_t flops      = (h_alpha != 1.0) ? 3.0 * nnz : 2.0 * nnz;
        flops             = (h_beta != 0.0) ? flops + m : flops;
        double gpu_gflops = flops / gpu_time_used / 1e6;
        size_t ell_mem    = dhyb->ell_nnz * (sizeof(rocsparse_int) + sizeof(T));
        size_t coo_mem    = dhyb->coo_nnz * (sizeof(rocsparse_int) * 2 + sizeof(T));
        size_t memtrans   = (m + n) * sizeof(T) + ell_mem + coo_mem;
        memtrans          = (h_beta != 0.0) ? memtrans + m : memtrans;
        double bandwidth  = memtrans / gpu_time_used / 1e6;

        printf("m\t\tn\t\tnnz\t\talpha\tbeta\tGFlops\tGB/s\tmsec\n");
        printf("%8d\t%8d\t%9d\t%0.2lf\t%0.2lf\t%0.2lf\t%0.2lf\t%0.2lf\n",
               m,
               n,
               dhyb->ell_nnz + dhyb->coo_nnz,
               h_alpha,
               h_beta,
               gpu_gflops,
               bandwidth,
               gpu_time_used);
    }

    return rocsparse_status_success;
}

#endif // TESTING_HYBMV_HPP
