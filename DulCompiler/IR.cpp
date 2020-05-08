//
//  IR.cpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 20.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#include "IR.hpp"
const char * IROP::typerepr[] = {
    "iadd",
    "iaddlm",
    "iaddrm",
    "isub",
    "isublm",
    "isubrm",
    "imul",
    "imullm",
    "imulrm",
    "idiv",
    "idivlm",
    "idivrm",
    "istore",
    "istorelm",
    "istorerm",
    "fadd",
    "faddlm",
    "faddrm",
    "fsub",
    "fsublm",
    "fsubrm",
    "fmul",
    "fmullm",
    "fmulrm",
    "fdiv",
    "fdivlm",
    "fdivrm",
    "fstore",
    "fstorelm",
    "fstorerm",
    "iret",
    "fret",
    "iretm",
    "fretm",
    "iwrite",
    "fwrite"
};
