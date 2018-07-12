/* ************************************************************************
 * Copyright 2018 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#include "utility.hpp"
#include "rocsparse.hpp"

// Level1
#include "testing_axpyi.hpp"
#include "testing_doti.hpp"
#include "testing_gthr.hpp"
#include "testing_gthrz.hpp"
#include "testing_roti.hpp"
#include "testing_sctr.hpp"

// Level2
#include "testing_coomv.hpp"
#include "testing_csrmv.hpp"
#include "testing_ellmv.hpp"
#include "testing_hybmv.hpp"

// Conversion
#include "testing_csr2coo.hpp"
#include "testing_csr2csc.hpp"
#include "testing_csr2ell.hpp"
#include "testing_csr2hyb.hpp"
#include "testing_coo2csr.hpp"
#include "testing_identity.hpp"
#include "testing_csrsort.hpp"

#include <iostream>
#include <stdio.h>
#include <boost/program_options.hpp>
#include <rocsparse.h>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    Arguments argus;
    argus.unit_check = 0;
    argus.timing     = 1;

    std::string function;
    char precision = 's';

    rocsparse_int device_id;

    po::options_description desc("rocsparse client command line options");
    desc.add_options()("help,h", "produces this help message")
        // clang-format off
        ("sizem,m",
         po::value<rocsparse_int>(&argus.M)->default_value(128),
         "Specific matrix size testing: sizem is only applicable to SPARSE-2 "
         "& SPARSE-3: the number of rows.")

        ("sizen,n",
         po::value<rocsparse_int>(&argus.N)->default_value(128),
         "Specific matrix/vector size testing: SPARSE-1: the length of the "
         "dense vector. SPARSE-2 & SPARSE-3: the number of columns")

        ("sizennz,z",
         po::value<rocsparse_int>(&argus.nnz)->default_value(32),
         "Specific vector size testing, LEVEL-1: the number of non-zero elements "
         "of the sparse vector.")

        ("mtx",
         po::value<std::string>(&argus.filename)->default_value(""), "read from matrix "
         "market (.mtx) format. This will override parameters m, n, and z.")

        ("laplacian-dim",
         po::value<rocsparse_int>(&argus.laplacian)->default_value(0), "assemble "
         "laplacian matrix for 2D unit square with dimension <dim>. This will override "
         "parameters m, n, z and mtx.")

        ("alpha", 
          po::value<double>(&argus.alpha)->default_value(1.0), "specifies the scalar alpha")
        
        ("beta", 
          po::value<double>(&argus.beta)->default_value(0.0), "specifies the scalar beta")
        
        ("function,f",
         po::value<std::string>(&function)->default_value("axpyi"),
         "SPARSE function to test. Options:\n"
         "  Level1: axpyi, doti, gthr, gthrz, roti, sctr\n"
         "  Level2: coomv, csrmv, ellmv, hybmv\n"
         "  Conversion: csr2coo, csr2csc, csr2ell,\n"
         "              csr2hyb, coo2csr\n"
         "  Sorting: csrsort")
        
        ("precision,r",
         po::value<char>(&precision)->default_value('s'), "Options: s,d")
        
        ("verify,v",
         po::value<rocsparse_int>(&argus.unit_check)->default_value(0),
         "Validate GPU results with CPU? 0 = No, 1 = Yes (default: No)")

        ("iters,i",
         po::value<rocsparse_int>(&argus.iters)->default_value(10),
         "Iterations to run inside timing loop")
        
        ("device",
         po::value<rocsparse_int>(&device_id)->default_value(0),
         "Set default device to be used for subsequent program runs");
    // clang-format on

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 0;
    }

    if(precision != 's' && precision != 'd')
    {
        fprintf(stderr, "Invalid value for --precision\n");
        return -1;
    }

    // Device Query
    rocsparse_int device_count = query_device_property();

    if(device_count <= device_id)
    {
        fprintf(stderr, "Error: invalid device ID. There may not be such device ID. Will exit\n");
        return -1;
    }
    else
    {
        set_device(device_id);
    }

    /* ============================================================================================
     */
    if(argus.M < 0 || argus.N < 0)
    {
        fprintf(stderr, "Invalid dimension\n");
        return -1;
    }

    // Level1
    if(function == "axpyi")
    {
        if(precision == 's')
            testing_axpyi<float>(argus);
        else if(precision == 'd')
            testing_axpyi<double>(argus);
    }
    else if(function == "doti")
    {
        if(precision == 's')
            testing_doti<float>(argus);
        else if(precision == 'd')
            testing_doti<double>(argus);
    }
    else if(function == "gthr")
    {
        if(precision == 's')
            testing_gthr<float>(argus);
        else if(precision == 'd')
            testing_gthr<double>(argus);
    }
    else if(function == "gthrz")
    {
        if(precision == 's')
            testing_gthrz<float>(argus);
        else if(precision == 'd')
            testing_gthrz<double>(argus);
    }
    else if(function == "roti")
    {
        if(precision == 's')
            testing_roti<float>(argus);
        else if(precision == 'd')
            testing_roti<double>(argus);
    }
    else if(function == "sctr")
    {
        if(precision == 's')
            testing_sctr<float>(argus);
        else if(precision == 'd')
            testing_sctr<double>(argus);
    }
    else if(function == "coomv")
    {
        if(precision == 's')
            testing_coomv<float>(argus);
        else if(precision == 'd')
            testing_coomv<double>(argus);
    }
    else if(function == "csrmv")
    {
        if(precision == 's')
            testing_csrmv<float>(argus);
        else if(precision == 'd')
            testing_csrmv<double>(argus);
    }
    else if(function == "ellmv")
    {
        if(precision == 's')
            testing_ellmv<float>(argus);
        else if(precision == 'd')
            testing_ellmv<double>(argus);
    }
    else if(function == "hybmv")
    {
        if(precision == 's')
            testing_hybmv<float>(argus);
        else if(precision == 'd')
            testing_hybmv<double>(argus);
    }
    else if(function == "csr2coo")
    {
        testing_csr2coo(argus);
    }
    else if(function == "csr2csc")
    {
        if(precision == 's')
            testing_csr2csc<float>(argus);
        else if(precision == 'd')
            testing_csr2csc<double>(argus);
    }
    else if(function == "csr2ell")
    {
        if(precision == 's')
            testing_csr2ell<float>(argus);
        else if(precision == 'd')
            testing_csr2ell<double>(argus);
    }
    else if(function == "csr2hyb")
    {
        if(precision == 's')
            testing_csr2hyb<float>(argus);
        else if(precision == 'd')
            testing_csr2hyb<double>(argus);
    }
    else if(function == "coo2csr")
    {
        testing_coo2csr(argus);
    }
    else if(function == "csrsort")
    {
        testing_csrsort(argus);
    }
    else
    {
        fprintf(stderr, "Invalid value for --function\n");
        return -1;
    }
    return 0;
}
