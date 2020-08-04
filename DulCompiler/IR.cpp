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
    "umin",
    "closure_get",
    "funcdef"
    
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
    FunctionalObject * _constructor;
    ObjectCreator(int size, FunctionalObject ** vt, FunctionalObject * cr){
        msize = size;
        caller = &constructor;
        vtable = vt;
        _constructor = cr;
        printf("constructor at %lld\n", this);
    }
};

IRFunction::StatVal constructor(void* data, void**stack){
    ObjectCreator * cr = (ObjectCreator*)data;
    void * object = calloc(1, cr->msize);
    ((FunctionalObject***)object)[0] = cr->vtable;
#if 1
    printf("allocated %d bytes at %p!\n", cr->msize, object);
#endif
    if(cr->_constructor){
#warning TODO: constructors
    }
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
    
    char meth_name [100];
    
    int table_size = countFunctionsWithinClass(classdef);
    FunctionalObject ** table = (FunctionalObject**)malloc(table_size * sizeof(void*));
    AstNode * class_body = classdef->children[1];
    FunctionalObject ** writer = table;
    for(auto entry: class_body->children){
        if(entry->t == AstNode::FUNCDEF){
            IRFunction * method = new IRFunction(entry->children[3]);
            sprintf(meth_name, "%s::%s.ir",  classdef->children[0]->memval, entry->children[0]->memval);
            ASMWriter wr;
            wr.writeIRFunction(method);
            method->print(meth_name);
            auto compiled = wr.generate();
            *writer = new FunctionalObject{method, compiled, entry};
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
            if(/* DISABLES CODE */ (1) || statement->val_type == &IntType){
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
            if(dest == -1)
                return namescope->indexOf(root->memval);
            else{
                
                operands.push_back(IROP{0, 0, IROP::namestore, dest, namescope->indexOf(root->memval)});
                return dest;
            }
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
        case AstNode::CLOSURE:{
            
            
            auto cl_coll = head->parent->children[5]->children;
            if(dest == -1){
                dest = currstackpos;
                currstackpos += 8;
            }
            for(int i = 0; i < cl_coll.size(); i++){
                if(strcmp(root->memval, cl_coll[i]->memval) == 0){
                    operands.push_back(IROP{0, 0, IROP::get_closure, dest, i});
                }
            }
            return dest;
        }break;
        case AstNode::FUNCDEF:{
            IRFunction * new_func = new IRFunction(root->children[3]);
            char name [100];
            sprintf(name, "%s_.ir", root->children[0]->memval);
            new_func->print(name);
            ASMWriter writer;
            writer.writeIRFunction(new_func);
            auto compiled = writer.generate();
            StatVal func_value = StatVal();
            vals.push_back(func_value);
            vals[vals.size() - 1].other = new FunctionalObject{new_func, compiled, root};
            int idx = (int)vals.size() - 1;
            if(dest == -1){
                dest = currstackpos;
                currstackpos += 8;
            }
            int clos = currstackpos;
            for(AstNode * closure: root->children[5]->children){
                operands.push_back(IROP{0, 0, IROP::namestore, currstackpos, namescope->indexOf(closure->memval)});
                currstackpos+=8;
            }
            
            
            operands.push_back(IROP{0, 0, IROP::funcdef, idx*8, dest, clos});
            return dest;
        }break;
        case AstNode::TUPLE:{
            dest = currstackpos;
            for(auto c:root->children){
                int dst = destOutASTLoad(c, currstackpos);
                if(dst != currstackpos){
                    operands.push_back(IROP{0, 0, IROP::namestore, currstackpos, dst});
                }
                currstackpos+=8;
            }
            return dest;
        }break;
        case AstNode::FCALL:{
            if(dest == -1){
                dest = currstackpos;
                currstackpos+=8;
            }
           
            if(root->children[0]->t == AstNode::AT){
				// method
                AstNode * method = root->children[0];
                int arg_place = destOutASTLoad(root->children[1], -1);
                int fplace = (*(LayoutType*)(*(LayoutType*)method->children[0]->val_type)["vtable"]).indexOf(method->children[1]->memval);
                operands.push_back(IROP{0, 0, IROP::method_call, dest, arg_place, fplace});
				
				
				
			} else {
				
                int fplace = destOutASTLoad(root->children[0], -1);
                int argplace = destOutASTLoad(root->children[1], -1);
				operands.push_back(IROP{0, 0, IROP::fcall, dest, argplace, fplace});
			}
            
            
            return dest;
        }break;
        case AstNode::CLASSDEF:{
            size_t classSize = ((LayoutType*)((*(LayoutType*)root->val_type)["return"]))->length()*8;
            StatVal classConstructor;
            vals.push_back(classConstructor);
            
            FunctionalObject ** vt =classToVtable(root);
            LayoutType * vtable_layout = (LayoutType*)(*(LayoutType*)root->val_type)["vtable"];
            int constr_idx = 0; //vtable_layout->indexOf("init");
            FunctionalObject * cons = 0;
            if(constr_idx != -1){
                cons = vt[constr_idx/8];
            }
            
            vals[vals.size() - 1].other = new ObjectCreator(classSize, vt, cons);
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
            
            int master_idx = ((LayoutType*)lhs->val_type)->indexOf(rhs->memval);
            if(master_idx == -1){
                master_idx = ((LayoutType*)(*(LayoutType*)lhs->val_type)["vtable"])->indexOf(rhs->memval);
            }
            if(master_idx == -1){
                throw SyntaxError{0, 0, "ddd"};
            }
            int master_place = destOutASTLoad(lhs, -1);
            operands.push_back(IROP{0, 0, IROP::at, dest, master_idx, master_place});
            return dest;
            
            
            
            
        }break;
        case AstNode::UMIN:{
            
        }break;
        case AstNode::SUBSCR:{
            AstNode * master = root->children[0];
            AstNode * arg = root->children[1];
            if(dest == -1){
                dest = currstackpos;
                currstackpos+=8;
            }
            LayoutType * vt = (LayoutType*)(*(LayoutType*)master->val_type)["vtable"];
            int at_idx = vt->indexOf("at");
            if(at_idx == -1){
                throw "not found at method at class";
            }
            int fa = destOutASTLoad(master, currstackpos);
            destOutASTLoad(arg, fa+8);
            
            operands.push_back(IROP{0, 0, IROP::method_call, dest, fa, at_idx});
            
            return dest;
            
            
        };
        default:
            break;
    }
    return -1;
}


