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
    static const uint8_t hdrs [];
    void writeMem(const uint8_t * source, int len){
        size_t demand = (memsize + len) * 1.5;
        if(demand >= memcap){
            mem = (uint8_t*)realloc(mem, memcap);
        }
        memcpy(mem+memsize, source, len);
        memsize += len;
        
    }
    void writeHeaders(){
        writeMem(hdrs, 10);
    }
    
    void writeFooter(){
        uint8_t mem []= {0x5d, 0xc3};
        writeMem(mem, sizeof(mem));
    }
public:
    class MemFactory{
        uint8_t * mem_template;
        int memsize;
        int val_idx;
        int val_size;
        
    public:
        MemFactory(uint8_t * mt, int size, int idx, int vsize){
            mem_template = (uint8_t*)malloc(size);
            memcpy(mem_template, mt, size);
            memsize = size;
            val_idx = idx;
            val_size = vsize;
        }
        
        MemFactory(std::initializer_list<uint8_t> templ,int idx, int vsize){
            mem_template = (uint8_t*)malloc(templ.size());
            memsize = (int)templ.size();
            int idx_ = 0;
            for(auto c: templ){
                mem_template[idx_] = c;
                idx_++;
            }
            val_idx = idx;
            val_size = vsize;
            
        }
        
        struct GeneratedMem{
            uint8_t * mem;
            int memsize;
            
            ~GeneratedMem(){
                free(mem);
            }
            
            GeneratedMem operator+(const GeneratedMem& rhs){
                uint8_t *new_mem = (uint8_t*)malloc(memsize + rhs.memsize);
                memcpy(new_mem, mem, memsize);
                memcpy(new_mem+memsize, rhs.mem, rhs.memsize);
                return GeneratedMem{new_mem, memsize + rhs.memsize};
            }
        };
        
        operator GeneratedMem(){
            return produce();
        }
        
        GeneratedMem operator() (void * p){
            return produce(p);
        }
        
        GeneratedMem produce(void* val_ptr){
            uint8_t * generated = (uint8_t*)malloc(memsize);
            memcpy(generated, mem_template, memsize);
            memcpy(generated+val_idx, val_ptr, val_size);
            return GeneratedMem{generated, memsize};
        }
        
        GeneratedMem produce(){
            assert(val_size == 0);
            return produce(0);
        }
        
        template<typename FP>
        GeneratedMem produceFunPtr(FP && fp){
            void * ptr = (void*)&fp;
            return produce(ptr);
        }
        
        
        ~MemFactory(){
            free(mem_template);
        }
        
    };
    
    void writeMem(const MemFactory::GeneratedMem& mem){
        writeMem(mem.mem, mem.memsize);
    }
    
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
    
    void inlineFunction(void*fp, int argc, int size, int frameofft, void ** data);
    
    ~ASMWriter(){
        free(mem);
    }
};


IRFunction::StatVal func_caller(void*, void**);
struct FunctionalObject{
    SuperCaller caller;
    IRFunction * context;
    ASMWriter::compiledFunc executable;
    
    FunctionalObject(IRFunction * ir, ASMWriter::compiledFunc f){
        context = ir;
        executable = f;
        caller = &func_caller;
    }
};




#endif /* ASM_hpp */
