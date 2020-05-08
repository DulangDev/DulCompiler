//
//  IR.hpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 20.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#ifndef IR_hpp
#define IR_hpp


#include <vector>
struct IROP{
    int lineno, linepos;
    enum types{
        iadd,
        iaddlm,
        iaddrm,
        isub,
        isublm,
        isubrm,
        imul,
        imullm,
        imulrm,
        idiv,
        idivlm,
        idivrm,
        istore,
        istorelm,
        istorerm,
        fadd,
        faddlm,
        faddrm,
        fsub,
        fsublm,
        fsubrm,
        fmul,
        fmullm,
        fmulrm,
        fdiv,
        fdivlm,
        fdivrm,
        fstore,
        fstorelm,
        fstorerm,
        iret,
        fret,
        iretm,
        fretm,
        iwrite,
        fwrite
        

    };
    types type;
    int first_arg;
    int second_arg;
    int third_arg;
    static const char * typerepr [];
};



#endif /* IR_hpp */
