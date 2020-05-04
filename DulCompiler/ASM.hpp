//
//  ASM.hpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 20.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#ifndef ASM_hpp
#define ASM_hpp

#include <stdio.h>
#include "IR.hpp"

class ASMWriter{
    uint8_t * mem;
    int memsize, memcap;
    struct stub{
        const char * symbol_name;
        int pos, size;
    };
    std::vector<stub> stubs;
    ASMWriter(const ASMWriter&) = delete;
    
    void writeMem(uint8_t * source, int len);
    void writeHeaders();
    
public:
    ASMWriter(){
        mem = (uint8_t*)malloc(1024);
        memcap = 1024;
        memsize = 0;
        writeHeaders();
    }
    
    void writeStub(int size, const char * name){
        for(int i = 0; i < size; i++){
            mem[memsize] = 0;
            memsize++;
        }
        stub st = {
            name, memsize - size, size
        };
        stubs.push_back(st);
    }
   
    void writeIROP(IROP op);
    void writeIRFunction(const ContextedFunctionIR*);
    
    ~ASMWriter(){
        free(mem);
    }
};

#endif /* ASM_hpp */
