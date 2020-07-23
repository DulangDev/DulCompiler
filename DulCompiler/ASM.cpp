//
//  ASM.cpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 20.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#include "ASM.hpp"
#include <sys/mman.h>


uint8_t inc_stack [] = {};
uint8_t dec_stack [] = {};
uint8_t callrax[] = {0xff, 0xd0};
uint8_t movabstorax [] = { 0x48, 0xb8};
uint8_t push_template []= {0x4D, 0x89, 0xF3, 0x49, 0x83, 0xC3, 0x00, 0x4D, 0x89, 0xDA};

ASMWriter::compiledFunc ASMWriter::generate() const {
    void * _mem = mmap(0, memsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_JIT, -1, 0);
    memcpy(_mem, mem, memsize);
    mprotect(_mem, memsize, PROT_READ | PROT_EXEC);
    return (ASMWriter::compiledFunc)(_mem);
}

void write_int(void * arg){
    printf("%lld\n", (int64_t*)arg);
}

IRFunction::StatVal (FunctionalObject::*executor_fo)(void**) = &FunctionalObject::operator();

void ASMWriter::writeIROP(IROP op){
    switch (op.type) {
        case IROP::vpush:{
            //push static value into r10
            uint8_t stat_to_r10 [] = {0x4d, 0x8b, 0x96, 0x88, 0x00, 0x00, 0x00};
            memcpy(stat_to_r10+3, &op.dest, 4);
            writeMem(stat_to_r10, sizeof(stat_to_r10));
            //push r10 into stack
            uint8_t r10_to_stack [] = {0x4d, 0x89, 0x97, 0x88, 0x00, 00, 00};
            memcpy(r10_to_stack+3, &op.farg, 4);
            writeMem(r10_to_stack, sizeof(r10_to_stack));
        }break;
        case IROP::iwrite:{
            // mov rdi, stack-place
            uint8_t mov_arg_to_rdi [] ={0x49, 0x8B, 0xBF, 0x88, 0x00, 0x00, 0x00};
            memcpy(mov_arg_to_rdi+3, &op.dest, 4);
            writeMem(mov_arg_to_rdi, sizeof(mov_arg_to_rdi));
            
            // mov func addr to rax
            
            uint8_t func_addr[8];
            void * fad = (void*)&write_int;
            memcpy(func_addr, &fad, 8);
            writeMem(movabstorax, sizeof(movabstorax));
            writeMem(func_addr, 8);
            // call func
            writeMem(callrax, sizeof(callrax));
            
            
        };
        case IROP::namestore:{
            uint8_t mover [] = { 0x4D, 0x8B, 0x57, 0x02, 0x4D, 0x89, 0x57, 0x01 };
            mover[3] = op.farg;
            mover[7] = op.dest;
            writeMem(mover, sizeof(mover));

        }break;
        case IROP::fcall:{
#warning TODO: args
            int framesize = op.farg;
            int dest = op.dest;
            int place = op.sarg;
            
            //mov rdi, [r15+place]
            uint8_t mov_arg[] = { 0x49, 0x8B, 0xBF, 0x88, 0x00, 0x00, 0x00 };
            memcpy(mov_arg+3, &place, 4);
            writeMem(mov_arg, sizeof(mov_arg));
            //mov rsi, stack + framesize
            uint8_t movr15torsi [] = { 0x4C, 0x89, 0xFE, 0x48, 0x81, 0xC6, 0x88, 0x00, 0x00, 0x00 };
            memcpy(movr15torsi+6, &framesize, 4);
            writeMem(movr15torsi, sizeof(movr15torsi));
            
            //movabstorax executor_fo
            writeMem(movabstorax, sizeof(movabstorax));
            writeMem((uint8_t*)&executor_fo, 8);
            //call rax
            writeMem(callrax, sizeof(callrax));
            //put rax to place in stack
            uint8_t res_to_stack [] = { 0x49, 0x89, 0x87, 0x88, 0x00, 0x00, 0x00 };
            memcpy(res_to_stack+3, &dest, 4);
            writeMem(res_to_stack, sizeof(res_to_stack));
            
        }break;
        case IROP::ret:{
            uint8_t cmd [] = { 0x49, 0x8B, 0x87, 0x88, 0x00, 0x00, 0x00, 0x5D, 0xC3 };
            memcpy(cmd+3, &op.dest, 4);
            writeMem(cmd, sizeof(cmd));
        }break;
        default:
            break;
    }
}
