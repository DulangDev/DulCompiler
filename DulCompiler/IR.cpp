//
//  IR.cpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 20.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#include "IR.hpp"
#include "ASM.hpp"
const char * IROP::typerepr[] = {
    "iadd",
    "isub",
    "imul",
    "idiv",
    "fadd",
    "fsub",
    "fmul",
    "fdiv",
    "vpush",
    "iwrite",
    "fwrite",
    "itof",
    "namestore",
    "return",
    "call",
    "at",
    "set_member",
    "method_call",
    "umin"
    
};

IRFunction::IRFunction(AstNode * func){
    head = func;
    namescope = head->getNamescope();
    writeBlock(func);
   
}

void IRFunction::writeBlock(AstNode *block){
    for(auto c: block->children){
        writeStatement(c);
    }
}


#warning TODO: GC!!!!!
struct ObjectCreator;
IRFunction::StatVal constructor (void*, void**);
struct ObjectCreator{
    SuperCaller caller;
    int msize;
    FunctionalObject ** vtable;
    ObjectCreator(int size, FunctionalObject ** vt){
        msize = size;
        caller = &constructor;
        vtable = vt;
    }
};

IRFunction::StatVal constructor(void* data, void**){
    ObjectCreator * cr = (ObjectCreator*)data;
    void * object = calloc(1, cr->msize);
    ((FunctionalObject***)object)[0] = cr->vtable;
    printf("allocated %d bytes at %p!\n", cr->msize, object);
    return IRFunction::StatVal{.other = object};
}


int countFunctionsWithinClass(AstNode * classdef){
    int counter = 0;
    for(int i = 0; i < classdef->children[1]->children.size(); i++){
        if(classdef->children[1]->children[i]->t == AstNode::FUNCDEF)
            counter++;
    }
    return counter;
}

FunctionalObject ** classToVtable(AstNode * classdef){
    int table_size = countFunctionsWithinClass(classdef);
    FunctionalObject ** table = (FunctionalObject**)malloc(table_size * sizeof(void*));
    AstNode * class_body = classdef->children[1];
    FunctionalObject ** writer = table;
    for(auto entry: class_body->children){
        if(entry->t == AstNode::FUNCDEF){
            IRFunction * method = new IRFunction(entry->children[3]);
            ASMWriter wr;
            wr.writeIRFunction(method);
            auto compiled = wr.generate();
            *writer = new FunctionalObject{method, compiled};
            writer++;
        }
    }
    return table;
}

static bool isLiteral(AstNode::type t){
    return t == AstNode::FLOATLIT || t == AstNode::STRLIT || t == AstNode::INTLIT || t == AstNode::_TRUE || t == AstNode::_FALSE;
}

void IRFunction::writeStatement(AstNode *statement){
    currstackpos = (int)namescope->size;
    switch (statement->t) {
        case AstNode::ASSIGN:{
            AstNode * lhs = statement->children[0];
            AstNode * rhs = statement->children[1];
            if(lhs->t == AstNode::NAME){
                int dest = namescope->indexOf(lhs->memval);
                int p = destOutASTLoad(rhs, -1);
                if(true){
                    operands.push_back(IROP{0, 0, IROP::namestore, dest, p});
                }
                
            }
            else if(lhs->t == AstNode::AT){
                
                
                
                int p = destOutASTLoad(rhs, -1);
                AstNode * root = lhs;
                int master_idx = ((LayoutType*)root->children[0]->val_type)->indexOf(root->children[1]->memval);
                int master_place = destOutASTLoad(root->children[0], -1);
                operands.push_back(IROP{0, 0, IROP::set_member, master_place, master_idx, p});
                
            }
        }break;
        case AstNode::WRITE:{
            if(1 || statement->val_type == &IntType){
                //iwrite
                int place = destOutASTLoad(statement->children[0], -1);
                operands.push_back(IROP{0, 0, IROP::iwrite, place});
            }
        }break;
        case AstNode::RET:{
            int place = destOutASTLoad(statement->children[0], -1);
            operands.push_back(IROP{0, 0, IROP::ret, place});
        }break;
        
        
        default:
            destOutASTLoad(statement, -1);
            break;
    }
    if(currstackpos+8 > maxframesize){
        maxframesize = currstackpos + 8;
    }
    
}

int IRFunction::destOutASTLoad(AstNode *root, int dest){
    
    switch (root->t) {
        case AstNode::ADD:
        case AstNode::SUB:
        case AstNode::MUL:
        case AstNode::DIV:{
            int typePrefix = (root->val_type == &FloatType)*4;
            int lhs = destOutASTLoad(root->children[0], -1);
            int rhs = destOutASTLoad(root->children[1], -1);
            IROP::types t = IROP::types((root->t - AstNode::ADD) + typePrefix);
            if(dest == -1){
                dest = currstackpos;
                currstackpos += 8;
            }
            operands.push_back(IROP{0, 0, t, dest, lhs, rhs});
            return dest;
        }break;
        case AstNode::FLOATLIT:
        case AstNode::INTLIT:
        case AstNode::STRLIT:
        case AstNode::_TRUE:
        case AstNode::_FALSE:{
            vals.push_back(StatVal{.other = (void*)root->memval});
            memcpy(&vals[vals.size() - 1].other, root->memval, 8);
            int idx = (int)vals.size() - 1;
            if(dest == -1){
                dest = currstackpos;
                currstackpos += 8;
            }
            operands.push_back(IROP{0, 0, IROP::vpush, idx*8, dest});
            return dest;
        }break;
        case AstNode::NAME:{
            
            return namescope->indexOf(root->memval);
        }break;
        case AstNode::TYPECAST:{
            int place = destOutASTLoad(root->children[0], -1);
            if(dest == -1){
                dest = currstackpos;
                currstackpos += 8;
            }
            operands.push_back(IROP{0, 0, IROP::itof, dest, place});
            return dest;
        }break;
        case AstNode::FUNCDEF:{
            IRFunction * new_func = new IRFunction(root->children[3]);
            new_func->print("ex.ir");
            ASMWriter writer;
            writer.writeIRFunction(new_func);
            auto compiled = writer.generate();
            StatVal func_value = StatVal();
            vals.push_back(func_value);
            vals[vals.size() - 1].other = new FunctionalObject{new_func, compiled};
            int idx = (int)vals.size() - 1;
            if(dest == -1){
                dest = currstackpos;
                currstackpos += 8;
            }
            operands.push_back(IROP{0, 0, IROP::vpush, idx*8, dest});
            return dest;
        }break;
        case AstNode::TUPLE:{
            dest = currstackpos;
            
            for(auto c:root->children){
                destOutASTLoad(c, currstackpos);
                currstackpos+=8;
            }
            return dest;
        }break;
        case AstNode::FCALL:{
            int fplace = destOutASTLoad(root->children[0], -1);
            int argplace = destOutASTLoad(root->children[1], -1);
            if(dest == -1){
                dest = currstackpos;
                currstackpos+=8;
            }
            operands.push_back(IROP{0, 0, IROP::fcall, dest, argplace, fplace});
            return dest;
        }break;
        case AstNode::CLASSDEF:{
            size_t classSize = ((LayoutType*)((*(LayoutType*)root->val_type)["return"]))->size;
            StatVal classConstructor;
            vals.push_back(classConstructor);
            
            
            vals[vals.size() - 1].other = new ObjectCreator(classSize, classToVtable(root));
            int idx = (int)vals.size() - 1;
            dest = namescope->indexOf(root->children[0]->memval);
            if(dest == -1){
                dest = currstackpos;
                currstackpos+=8;
            }
            operands.push_back(IROP{0, 0, IROP::vpush, idx*8, dest});
            return dest;
            
        }break;
        case AstNode::AT:{
            if(dest == -1){
                dest = currstackpos;
                currstackpos+=8;
            }
            AstNode * rhs = root->children[1];
            AstNode * lhs = root->children[0];
            if(rhs->t == AstNode::FCALL){
                //method call
                AstNode * method_name = rhs->children[0];
                AstNode * args = rhs->children[1];
                LayoutType * vtable = (LayoutType*)(*(LayoutType*)lhs->val_type)["vtable"];
                int func_idx = (*vtable).indexOf(method_name->memval);
                int this_arg = destOutASTLoad(lhs, -1);
                if(lhs->t == AstNode::NAME){
                    operands.push_back(IROP{0, 0, IROP::namestore, currstackpos, this_arg});
                }
                destOutASTLoad(args, -1);
                operands.push_back(IROP{0, 0, IROP::method_call, dest, func_idx, currstackpos});
                return dest;
            } else {
                int master_idx = ((LayoutType*)lhs->val_type)->indexOf(rhs->memval);
                int master_place = destOutASTLoad(lhs, -1);
                operands.push_back(IROP{0, 0, IROP::at, dest, master_idx, master_place});
                return dest;
            }
            
            
            
        }break;
        case AstNode::UMIN:{
            
        }break;
        default:
            break;
    }
    return -1;
}


