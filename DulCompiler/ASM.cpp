//
//  ASM.cpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 20.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#include "ASM.hpp"
#include <sys/mman.h>
#include <assert.h>


uint8_t inc_stack [] = {};
uint8_t dec_stack [] = {};
uint8_t callrax[] = {0xff, 0xd0};
uint8_t movabstorax [] = { 0x48, 0xb8};
uint8_t push_template []= {0x4D, 0x89, 0xF3, 0x49, 0x83, 0xC3, 0x00, 0x4D, 0x89, 0xDA};


ASMWriter::MemFactory StatToR10         ({0x4d, 0x8b, 0x96, 0x00, 0x00, 0x00, 0x00}, 3, 4);
ASMWriter::MemFactory StackToR11        ({0x4D, 0x8B, 0x9F, 0x88, 0x00, 0x00, 0x00}, 3, 4);
ASMWriter::MemFactory StackToR10        ({0x4D, 0x8B, 0x97, 0x00, 0x00, 0x00, 0x00}, 3, 4);
ASMWriter::MemFactory R10ToStack        ({0x4d, 0x89, 0x97, 0x00, 0x00, 00, 00}, 3, 4);
ASMWriter::MemFactory StackToRDI        ({0x49, 0x8B, 0xBF, 0x88, 0x00, 0x00, 0x00}, 3, 4);
ASMWriter::MemFactory MovAbsToRax       ({0x48, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0}, 2, 8);
ASMWriter::MemFactory StackToRax        ({0x49, 0x8B, 0x87, 0x88, 0x00, 0x00, 0x00}, 3, 4);
ASMWriter::MemFactory DerefRax          ({0x48, 0x8B, 0x00}, 0, 0);
ASMWriter::MemFactory CallRax           ({0xff, 0xd0}, 0, 0);
ASMWriter::MemFactory StackShiftToRSI   ({0x4C, 0x89, 0xFE, 0x48, 0x81, 0xC6, 0x88, 0x00, 0x00, 0x00}, 6, 4);
ASMWriter::MemFactory RaxToStack        ({0x49, 0x89, 0x87, 0x88, 0x00, 0x00, 0x00}, 3, 4);
ASMWriter::MemFactory ReturnVal         ({0x49, 0x8B, 0x87, 0x88, 0x00, 0x00, 0x00, 0x5D, 0xC3, 0x41, 0x5D}, 3, 4);
ASMWriter::MemFactory R11DertoR10       ({0x4D, 0x8B, 0x93, 00, 0x00, 0x00, 0x00}, 3, 4);
ASMWriter::MemFactory R10dertoR10       ({0x4D, 0x8B, 0x92, 0x00, 0x00, 0x00, 0x00}, 3, 4);
ASMWriter::MemFactory R10dertoRax       ({0x49, 0x8B, 0x82, 0x00, 0x00, 0x00, 0x00}, 3, 4);
ASMWriter::MemFactory R11toRDI          ({0x4C, 0x89, 0xDF}, 0, 0);
ASMWriter::MemFactory R11toRax          ({0x4C, 0x89, 0xD8}, 0, 0);
ASMWriter::MemFactory R10dertoR11       ({0x4D, 0x8B, 0x9A, 0x88, 0x00, 0x00, 0x00}, 3, 4);
ASMWriter::MemFactory R11dertoR11       ({0x4D, 0x8B, 0x9B, 0x88, 0x00, 0x00, 0x00}, 3, 4);
ASMWriter::MemFactory RetStack          ({0x49, 0x81, 0xEF, 0x80, 0x00, 0x00, 0x00}, 3, 4);
ASMWriter::MemFactory ClosToR10         ({0x4D, 0x8B, 0x95, 0x88, 0x00, 0x00, 0x00}, 3, 4);
//ASMWriter::MemFactory RaxToStack        ({0x49, 0x89, 0x87, 0x88, 0x00, 0x00, 0x00}, 3, 4);

//takes as parameter register no

void write_int(void * arg){
    printf("%lld\n", (int64_t)arg);
}
#define PRINTER_REG_R10 0xD7
#define PRINTER_REG_R11 0xDF
#define PRINTER_REG_R13 0xEF
#define PRINTER_REG_RAX 0xc7
ASMWriter::MemFactory::GeneratedMem printTempregister(uint8_t Rno){
    auto RegToRDI = new ASMWriter::MemFactory ({ 0x4C, 0x89, 0xD7 }, 2, 1);
    if(Rno == PRINTER_REG_RAX){
        RegToRDI = new ASMWriter::MemFactory({ 0x48, 0x89, 0xC7 }, 2, 1);
    }
    return (*RegToRDI)(&Rno) + MovAbsToRax.produceFunPtr(&write_int) + CallRax;
}
#ifndef MAP_JIT
#define MAP_JIT 0
#endif
ASMWriter::compiledFunc ASMWriter::generate() const {
    void * _mem = mmap(0, memsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_JIT, -1, 0);
    memcpy(_mem, mem, memsize);
    mprotect(_mem, memsize, PROT_READ | PROT_EXEC);
    printf("function compiled at %p: ", _mem);
    for(int i = 0; i < memsize; i++){
        uint8_t u = mem[i];
        printf("%02X", u);
    }
    printf("\n");
    
    
    return (ASMWriter::compiledFunc)(_mem);
}



//IRFunction::StatVal (FunctionalObject::*executor_fo)(void**) = &FunctionalObject::operator();

void ASMWriter::writeIROP(IROP op){
    switch (op.type) {
        case IROP::vpush:{
            //push static value into r10
            uint8_t val_mem [8];
            memcpy(val_mem, (void**)f->vals.data()+ (op.dest / 8), 8);
            writeMem(MovAbsToRax(val_mem));
            writeMem(RaxToStack(&op.farg));
        }break;
        case IROP::iwrite:{
            // mov rdi, stack-place
            writeMem(StackToRDI(&op.dest));
            // mov func addr to rax
            writeMem(MovAbsToRax.produceFunPtr(&write_int));
            // call func
            writeMem(CallRax);
        };
        case IROP::namestore:{
            writeMem(StackToR10(&op.farg));
            writeMem(R10ToStack(&op.dest));
        }break;
        case IROP::fcall:{

            int framesize = op.farg;
            int dest = op.dest;
            int place = op.sarg;
            
            //mov rdi, [r15+place]
            writeMem(StackToRDI(&place));
            //mov rsi, stack + framesize
            writeMem(StackShiftToRSI(&framesize));
            //mov this to rax
            writeMem(StackToRax(&place));
        
            writeMem(DerefRax);
            
            //call rax
            writeMem(CallRax);
            //put rax to place in stack
            writeMem(RaxToStack(&dest));
            //writeMem(RetStack(&framesize));
        }break;
        case IROP::ret:{
            writeMem(ReturnVal(&op.dest));
        }break;
        case IROP::set_member:{
            //first of all put this into r11
            writeMem(StackToR11(&op.dest));
            // then put value into r10
            writeMem(StackToR10(&op.sarg));
            // at last write r10 into [r11 + offset]
            // less common operation thus declare template here
            ASMWriter::MemFactory R10toDerR11 ({0x4D, 0x89, 0x93, 0x88, 0x00, 0x00, 0x00}, 3, 4);
            writeMem(R10toDerR11(&op.farg));
        }break;
        case IROP::at:{
            //first we put this ptr to r11
            writeMem(StackToR11(&op.sarg));
            //then we put into r10 value at [r11 + offset]
            
           
            writeMem(R11DertoR10(&op.farg));
            //then we put r10 to stack
            writeMem(R10ToStack(&op.dest));
            
        }break;
        case IROP::method_call:{
            // here we have op.dest for result putting
            // op.farg as stack-arg-offset
            // op.sarg as offset in vtable
            writeMem(StackToR10(&op.farg));
            int vt_offset = 0;
            writeMem(R10dertoR11(&vt_offset));
            writeMem(R11dertoR11(&op.sarg));
            writeMem(R11toRDI);
            writeMem(R11toRax);
            writeMem(StackShiftToRSI(&op.farg));
            writeMem(DerefRax);
            writeMem(CallRax);
            if(op.dest != -1)
            writeMem(RaxToStack(&op.dest));
           
           
          
        }break;
        case IROP::get_closure:{
            
            writeMem(ClosToR10(&op.farg));
            writeMem(R10ToStack(&op.dest));
        }break;
        case IROP::funcdef:{
            int push_to = op.farg;
            int fo_pos = op.dest;
            int closures = op.sarg;
            MemFactory MovAbsToRdi({ 0x48, 0xBF, 0x31, 0x09, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00 }, 2, 8);
            void * p = ((void**)f->vals.data())[fo_pos/8];
            writeMem(MovAbsToRdi(&p));
            writeMem(StackShiftToRSI(&closures));
            writeMem(MovAbsToRax.produceFunPtr(&FunctionalObject::inferClosures));
            writeMem(CallRax);
            writeMem(RaxToStack(&push_to));
            
        }break;
        default:
            break;
    }
}

void ASMWriter::inlineFunction(void *fp, int argc, int size, int frameofft, void ** data){
    size -= 2 + sizeof(10);
    uint8_t * func_mem = (uint8_t*)fp + sizeof(10);
    
}

IRFunction::StatVal func_caller(void*data, void**stack){
    FunctionalObject * fo = (FunctionalObject*)data;
    printf("Executing function at %p\n", fo->executable);
    return fo->executable(stack, (void**)fo->context->vals.data(), fo->closures);
}
