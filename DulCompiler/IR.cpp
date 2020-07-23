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
    "call"
    
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
        case AstNode::FCALL:{
#warning TODO: args
            
            int fplace = destOutASTLoad(root->children[0], -1);
            int argplace = destOutASTLoad(root->children[1], -1);
            if(dest == -1){
                dest = currstackpos;
                currstackpos+=8;
            }
            operands.push_back(IROP{0, 0, IROP::fcall, dest, this->maxframesize, fplace});
            return dest;
        }break;
        default:
            break;
    }
    return -1;
}


