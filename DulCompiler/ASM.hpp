//
//  ASM.hpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 20.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//


/*
* We store stackptr into r15 and static into r14
* real stack pointer is stored into r13
*
*/
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
    
    void writeMem(uint8_t * source, int len){
        size_t demand = (memsize + len) * 1.5;
        if(demand >= memcap){
            mem = (uint8_t*)realloc(mem, memcap);
        }
        memcpy(mem+memsize, source, len);
        memsize += len;
        
    }
    void writeHeaders(){
        uint8_t _mem [] = {
            0x55, 0x49, 0x89, 0xFF, 0x49, 0x89, 0xF6, 0x49, 0x89, 0xfd

        };
        writeMem(_mem, sizeof(mem));
    }
    
    void writeFooter(){
        uint8_t mem []= {0x5d, 0xc3};
        writeMem(mem, sizeof(mem));
    }
    
    
public:
    using compiledFunc = IRFunction::StatVal(*)(void**stack, void**data);
    compiledFunc generate() const;
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
    void writeIRFunction(const IRFunction * func){
        for(auto op: func->operands){
            writeIROP(op);
        }
        writeFooter();
    }
    
    ~ASMWriter(){
        free(mem);
    }
};
struct FunctionalObject{
    IRFunction * context;
    ASMWriter::compiledFunc executable;
    IRFunction::StatVal operator()(void ** stack){
        return executable(stack, (void**)context->vals.data());
    }
};


#endif /* ASM_hpp */
