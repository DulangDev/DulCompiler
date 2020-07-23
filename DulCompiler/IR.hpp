//
//  IR.hpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 20.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#ifndef IR_hpp
#define IR_hpp

#include "Type.hpp"
#include "AST.hpp"
#include <vector>
#include "BuiltIn/bin_str.hpp"
struct IROP{
    int lineno, linepos;
    enum types{
        iadd,
        isub,
        imul,
        idiv,
        fadd,
        fsub,
        fmul,
        fdiv,
        vpush,
        iwrite,
        fwrite,
        itof,
        namestore,
        ret,
        fcall
        

    };
    types type;
    int dest, farg, sarg;
    static const char * typerepr [];
};

struct IRFunction{
    LayoutType * namescope;
    AstNode * head;
    std::vector<IROP> operands;
    union StatVal {
        double fval;
        int64_t ival;
        bool bval;
        void * other;
    };
    
    std::vector<StatVal> vals;
    
    //stackpos is counted from end of variable layout
    //static values are to be inside executable memory but here are presented in statics
    int currstackpos=0;
    int maxframesize=0;
    IRFunction(AstNode * func);
    int destOutASTLoad(AstNode * root, int dest);
    //returns place in stack
    void writeBlock(AstNode * block);
    void writeStatement(AstNode * statement);
    int getValue(AstNode * v);
    
    void print(const char * name){
        FILE * f = fopen(name, "w");
        
        for(int i = 0; i < namescope->length(); i++){
            LayoutType::entry e = namescope->getEntry(i);
            fprintf(f, "var %s of type %s\n", e.name, e.t->name);
        }
        fprintf(f, "code: --------\n");
        for(auto op: operands){
            fprintf(f, "%.10s %d %d %d\n", IROP::typerepr[op.type], op.dest, op.farg, op.sarg);
        }
        fclose(f);
    }
    
};



#endif /* IR_hpp */
