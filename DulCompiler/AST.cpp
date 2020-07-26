//
//  AST.cpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 18.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#include "AST.hpp"
#define COMP_ERR 1

AstNode* AstNode::parseFile(const char * name){
    LexemScanner scanner(name);
    lexIter iter = scanner.getIterator();
    try{
        return parseCompound(iter);
    } catch(SyntaxError err){
        fprintf(stderr, "Syntax error at %d:%d  %s\n", err.lineno, err.linepos, err.message);
        return 0;
    }
}



void setFunctionParenthesisDownwalk(AstNode* node, AstNode * f){
    if(node->t == AstNode::FUNCDEF){
        node->add(f);
        node->add(new AstNode(AstNode::TUPLE, {}));
        f = node;
        setFunctionParenthesisDownwalk(node->children[3], f);
        return;
    }
    if(node->children.size())
    for(auto c: node->children){
        if(c)
        setFunctionParenthesisDownwalk(c, f);
    }
}

Type*& recursive_clos_lookup(AstNode * node, const char * name){
    return undefined;
}

Type*& clos_lookup(AstNode * node, const char * name){
    return undefined;
}



AstNode* AstNode::parseCompound(lexIter& iter){
    AstNode * block = new AstNode(BLOCK);
    while(not iter->isClosing()){
            AstNode * statement = parseStatement(iter);
            block->add(statement);
    }
    return block;
}
AstNode* AstNode::parseStatement(lexIter& iter){
    // a statement can be either
    // a write statement
    // an assign (further also inplace assign)
    // funccall or methodcall which is kind of funccall
    // also classdef interfacedef and funcdef are statements
    // upd if statement, while and for
    switch (iter->lexType) {
        case Lexem::KWWRITE:{
            iter++;
            AstNode * expr_part = parseExpr(iter);
            iter++;
            return new AstNode(WRITE, {expr_part});
        }
        case Lexem::EOL:{
            iter++;
            return new AstNode(EMPTY);
        }break;
        case Lexem::KWRET:{
            iter++;
            AstNode * expression;
            if(iter->lexType != Lexem::EOL)
                expression = parseExpr(iter);
            else
                expression = new AstNode(EMPTY);
            iter++;
            return new AstNode(RET, {expression});
            
        }
        case Lexem::KWIF:{
            iter++;
            return parseIf(iter);
        }break;
        case Lexem::KWWHILE:{
            iter++;
            return parseWhile(iter);
        }break;
        case Lexem::KWFOR:{
            iter++;
            return parseFor(iter);
        }break;
        case Lexem::KWCLASS:{
            iter++;
            return parseClassDef(iter);
        }break;
        case Lexem::KWINTERFACE:{
            iter++;
            return parseInterface(iter);
        }break;
        case Lexem::KWFUN:{
            iter++;
            return parseFuncDef(iter);
        }
        default:{
            AstNode * lval = parseExpr(iter);
            
            if(lval->t == TYPEDECL || lval->t == FUNTYPEDECL){
                return lval;
            }
            if(lval->t == FCALL){
                return lval;
            }
            
            Lexem next = *iter;
            if(next.lexType != Lexem::ASSIGN && lval->t != AT){
                throw SyntaxError{next.lineno, next.linepos, (char*)"expected ="};
            }
            iter++;
            AstNode* rval;
            if(iter->lexType == Lexem::KWFUN){
                iter++;
                rval = parseFuncDef(iter);
            } else
            rval = parseExpr(iter);
            if(next.lexType != Lexem::ASSIGN && !(rval->t == FCALL && lval->t == AT)){
                throw SyntaxError{next.lineno, next.linepos, (char*)"expected ="};
            } 
            iter++;
            return new AstNode(ASSIGN, {lval, rval});
        }break;
    }
    throw SyntaxError{iter->lineno, iter->linepos, (char*)"statement required"};
    
    
}
AstNode* AstNode::parseExpr(lexIter& i){
    AstNode * first = parseAs(i);
    if(i->lexType == Lexem::COMMA){
        first = new AstNode(TUPLE, {first});
        while(i->lexType == Lexem::COMMA){
            i++;
            AstNode * cur = parseAs(i);
            first->add(cur);
        }
    }
    return first;
}

AstNode * AstNode::parseAs(lexIter &i){
    AstNode * expr = parseLogOr(i);
    if(i->lexType == Lexem::AS){
        i++;
        AstNode * type = parseSuffix(i);
        return new AstNode(TYPECAST, {expr, type});
        
    }
    return expr;
}

AstNode* AstNode::parseLogOr(lexIter& i){
    AstNode * leftmost = parseLogAnd(i);
    while(i->lexType == Lexem::KWOR){
        i++;
        AstNode * righter = parseLogAnd(i);
        leftmost = new AstNode(OR, {leftmost, righter});
    }
    return leftmost;
}
AstNode* AstNode::parseLogAnd(lexIter& i){
    AstNode * leftmost = parseLogNot(i);
    while(i->lexType == Lexem::KWAND){
        i++;
        AstNode * righter = parseLogNot(i);
        leftmost = new AstNode(OR, {leftmost, righter});
    }
    return leftmost;
}
AstNode* AstNode::parseLogNot(lexIter& i){
    if(i->lexType == Lexem::KWNOT){
        i++;
        AstNode * expr = parseComp(i);
        return new AstNode(NOT, {expr});
    }
    return parseComp(i);
}
AstNode* AstNode::parseComp(lexIter& i){
    AstNode * leftmost = parseIn(i);
    while(i->lexType == Lexem::EQ || i->lexType == Lexem::LEQ || i->lexType == Lexem::LESS || i->lexType == Lexem::GEQ || i->lexType == Lexem::GREATER || i->lexType == Lexem::NEQ){
        Lexem::type t = i->lexType;
        i++;
        AstNode * righter = parseIn(i);
        leftmost = new AstNode(AstNode::type(t - Lexem::EQ+EQUALS), {leftmost, righter});
    }
    return leftmost;
}
AstNode* AstNode::parseIn(lexIter& i){
    AstNode * left = parsePlusMinus(i);
    if(i->lexType == Lexem::KWIN){
        i++;
        AstNode * right = parsePlusMinus(i);
        left = new AstNode(IN, {left, right});
    }
    return left;
}
AstNode* AstNode::parsePlusMinus(lexIter& i){
    AstNode * left = parseMultDiv(i);
    while( i->lexType == Lexem::PLUS || i->lexType == Lexem::MINUS ){
        Lexem::type t = i->lexType;
        i++;
        AstNode * right = parseMultDiv(i);
        left = new AstNode(AstNode::type(ADD + t - Lexem::PLUS), {left, right});
    }
    return left;
}
AstNode* AstNode::parseMultDiv(lexIter& i){
    AstNode * left = parseUnary(i);
    while( i->lexType == Lexem::ASTERISK || i->lexType == Lexem::SLASH ){
        Lexem::type t = i->lexType;
        i++;
        AstNode * right = parseUnary(i);
        left = new AstNode(AstNode::type(MUL + t - Lexem::ASTERISK), {left, right});
    }
    return left;
}
AstNode* AstNode::parseUnary(lexIter& i){
    switch (i->lexType) {
        case Lexem::PLUS:{
            i++;
            AstNode * expr = parseSuffix(i);
            return new AstNode(UPLUS, {expr});
        }break;
        case Lexem::MINUS:{
            i++;
            AstNode * expr = parseSuffix(i);
            return new AstNode(UMIN, {expr});
            
        }break;
        //therefore we know all field names of this we can omit explicit . operator
        /*case Lexem::DOT:{
            
        }break;*/
        default:
            return parseSuffix(i);
            break;
    }
}
AstNode* AstNode::parseSuffix(lexIter& i){
    AstNode * left = parseTopLevel(i);
    while(1){
        if(i->lexType == Lexem::OP_R_BR){
            i++;
            AstNode * expr;
            if(i->lexType == Lexem::CL_R_BR){
                expr = new AstNode(EMPTY);
            } else expr = parseExpr(i);
            if(i->lexType != Lexem::CL_R_BR){
                throw SyntaxError{i->lineno, i->linepos, (char*)"expected ) bracket here"};
            }
            i++;
            if(expr->t != TUPLE){
				AstNode * l_a = expr;
				expr = new AstNode(TUPLE);
				if(l_a->t != EMPTY){
					expr->add(l_a);
				}
			}
            if(left->t == AT){
				std::vector<AstNode*> expanded_args;
				expanded_args.push_back(left->children[0]);
				for(auto arg: expr->children){
					expanded_args.push_back(arg);
				}
                expr->children = expanded_args;
                for(auto c : expanded_args){
                    c->parent = expr;
                }
			}
            left = new AstNode(FCALL, {left, expr});
            
           
        } else if(i->lexType == Lexem::OP_S_BR){
            i++;
            AstNode * expr;
            if(i->lexType == Lexem::CL_S_BR){
                expr = new AstNode(EMPTY);
            } else expr = parseExpr(i);
            if(i->lexType != Lexem::CL_S_BR){
                throw SyntaxError{i->lineno, i->linepos, (char*)"expected ] bracket here"};
            }
            i++;
            left =  new AstNode(SUBSCR, {left, expr});
        } else if(0){
            i++;
            AstNode * rhs = parseSuffix(i);
            left =  new AstNode(AT, {left, rhs});
        } else if(i->lexType == Lexem::COLON){
            i++;
            AstNode * type = parseSuffix(i);
            return new AstNode(TYPEDECL, {left, type});
        } else if(i->lexType == Lexem::ARROW){
            i++;
            AstNode * _t = parseSuffix(i);
            left =  new AstNode(FUNTYPEDECL, {left, _t});
        } else return left;
    }
    
    return left;
}
AstNode* AstNode::parseTopLevel(lexIter& i){
	AstNode * expr;
    switch (i->lexType) {
        case Lexem::OP_R_BR:{
            i++;
            if(i->lexType == Lexem::CL_R_BR){
                expr = new AstNode(EMPTY);
                i++;
            } else {expr = parseExpr(i);
            if(i->lexType != Lexem::CL_R_BR){
                throw SyntaxError{i->lineno, i->linepos, (char*)"expected ) bracket here"};
            }
                i++;
            }
        }break;
        case Lexem::NAME:{
            expr = new AstNode(NAME);
            strcpy(expr->memval, i->val.strval);
            i++;
        }break;
        case Lexem::INTLIT:{
            expr = new AstNode(INTLIT);
            memcpy(expr->memval, &i->val.ival, sizeof(int64_t));
            i++;
        }break;
        case Lexem::FLOATLIT:{
            expr = new AstNode(FLOATLIT);
            memcpy(expr->memval, &i->val.fval, sizeof(double));
            i++;
        }break;
        case Lexem::STRLIT:{
            expr  = new AstNode(STRLIT);
            memcpy(&expr->memval, &i->val.strval, sizeof(char*));
            i++;
        }break;
        case Lexem::KWTRUE:{
            i++;
            expr =  new AstNode(_TRUE);
        }break;
        case Lexem::KWFALSE:{
            i++;
            expr =  new AstNode(_FALSE);
        }break;
        default:
            throw SyntaxError{i->lineno, i->linepos, (char*)"top level expection expected"};
            break;
    }
    if(i->lexType == Lexem::DOT){
		i++;
		AstNode * rhs = parseTopLevel(i);
		expr = new AstNode(AT, {expr, rhs});
	}
    return expr;
}
AstNode* AstNode::parseClassDef(lexIter& i){
    AstNode * classdef = new AstNode(CLASSDEF);
    AstNode * name = parseTopLevel(i);
    classdef->add(name);
    if(i->lexType == Lexem::KWHAS){
        i++;
        AstNode * extending = parseExpr(i);
        classdef->add(extending);
    }
    if(i->lexType != Lexem::OP_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected { at class declaration"};
    }
    i++;
    AstNode * block = parseCompound(i);
    if(i->lexType != Lexem::CL_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected } at end of class declaration"};
    }
    i++;
    classdef->add(block);
    return classdef;
}
AstNode* AstNode::parseFuncDef(lexIter& i){
    //kyeword fun is skipped already
    AstNode * fundecl = new AstNode(FUNCDEF);
    AstNode * name = new AstNode(EMPTY);
    if(i->lexType != Lexem::OP_R_BR)
        name = parseTopLevel(i);
    if(i->lexType != Lexem::OP_R_BR){
        throw SyntaxError{i->lineno, i->linepos, "( expected at function declaration"};
    }
    i++;
    AstNode * args;
    if(i->lexType != Lexem::CL_R_BR)
         args = parseExpr(i);
    else
        args = new AstNode(EMPTY);
    if(i->lexType != Lexem::CL_R_BR){
        throw SyntaxError{i->lineno, i->linepos, ") expected after parameter list"};
    }
    fundecl->add(name);
    fundecl->add(args);
    i++;
    if(i->lexType != Lexem::OP_BRACE){
        //return type given
        AstNode * ret = parseSuffix(i);
        fundecl->add(ret);
    } else {
        fundecl->add(new AstNode(EMPTY));
    }
    if(i->lexType != Lexem::OP_BRACE){
        //think of it as abstract fundecl
        return fundecl;
    }
    i++;
    AstNode * body = parseCompound(i);
    if(i->lexType != Lexem::CL_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected } at end of function body"};
    }
    i++;
    fundecl->add(body);
    return fundecl;
}


AstNode* AstNode::parseInterface(lexIter&i){
    AstNode * idef = new AstNode(INTERFACE);
    AstNode * name = parseTopLevel(i);
    idef->add(name);
    if(i->lexType != Lexem::OP_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected { at class declaration"};
    }
    i++;
    AstNode * block = parseCompound(i);
    if(i->lexType != Lexem::CL_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected } at end of class declaration"};
    }
    i++;
    idef->add(block);
    return idef;
}

const char * AstNode::typerepr [] = {
    "RET",
    "WRITE",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "ASSIGN",
    "EQUALS",
    "NEQ",
    "GEQ",
    "LEQ",
    "GREATER",
    "LESS",
    
    "NOT",
    "AND",
    "OR",
    "IF",
    "WHILE",
    "FOR",
    "FUNCDEF",
    "CLASSDEF",
    "FCALL",
    "NAME",
    "STRLIT",
    "FLOATLIT",
    "INTLIT",
    "AT",
    "TYPEDECL",
    "TUPLE",
    "BLOCK",
    "IN",
    "UMIN",
    "UPLUS",
    "NOP",
    "TRUE",
    "FALSE",
    "INTERFACE",
    "SUBSCR",
    "TYPECAST",
    "FUNTYPEDECL"
};

AstNode* AstNode::parseIf(lexIter& i){
    AstNode * cond_expr = parseExpr(i);
    if(i->lexType != Lexem::OP_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected { after if statement"};
    }
    i++;
    AstNode * if_block = parseCompound(i);
    if(i->lexType != Lexem::CL_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected } after if body statement"};
    }
    i++;
    AstNode * ifstat = new AstNode(IF, {cond_expr, if_block});
    if(i->lexType == Lexem::KWELSE){
        i++;
        if(i->lexType != Lexem::OP_BRACE){
            throw SyntaxError{i->lineno, i->linepos, "expected { after else statement"};
        }
        AstNode * elseblock = parseCompound(i);
        if(i->lexType != Lexem::CL_BRACE){
            throw SyntaxError{i->lineno, i->linepos, "expected } after else body statement"};
        }
        i++;
        ifstat->add(elseblock);
        
    }
    return ifstat;
}
AstNode* AstNode::parseFor(lexIter&i){
    AstNode * iter_name = parseTopLevel(i);
    if(i->lexType != Lexem::KWIN){
        throw SyntaxError{i->lineno, i->linepos, "expected in after for iterator name"};
    }
    i++;
    AstNode * coll = parseExpr(i);
    if(i->lexType != Lexem::OP_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected { after for"};
    }
    i++;
    AstNode * body = parseCompound(i);
    if(i->lexType != Lexem::CL_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected } after for"};
    }
    i++;
    return new AstNode(FOR, {iter_name, coll, body});
}
AstNode* AstNode::parseWhile(lexIter&i){
    AstNode * while_expr = parseExpr(i);
    if(i->lexType != Lexem::OP_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected { after while"};
    }
    i++;
    AstNode * body = parseCompound(i);
    if(i->lexType != Lexem::CL_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected } after while"};
    }
    i++;
    return new AstNode(WHILE, {while_expr, body});
}

void AstNode::parseMethodDecl(Type * this_type){
    
    AstNode * this_arg = new AstNode(NAME);
    memcpy(this_arg->memval, "this", 5);
    this_arg->val_type = this_type;
    AstNode * this_name = new AstNode(NAME);
    this_name->val_type = this_type;
    strcpy(this_name->memval, this_type->name);
    AstNode * this_arg_wrapper = new AstNode(TYPEDECL, {this_arg, this_name});
    this_arg_wrapper->val_type = this_type;
    
    if(children[1]->t == TYPEDECL){
        //only one arg
        children[1] = new AstNode(TUPLE, {this_arg_wrapper, children[1]});
    } else if(children[1]->t == TUPLE){
        //tuple
        std::vector<AstNode*> expanded_args;
        expanded_args.push_back(this_arg_wrapper);
        for(auto arg:children[1]->children){
            expanded_args.push_back(arg);
        }
        children[1]->children = expanded_args;
        children[1]->inferTypes();
    } else {
        //null args
        children[1] = this_arg_wrapper;
    }
    
    parseFundecl(0);
}


void AstNode::parseFundecl(LayoutType * os) {
    LayoutType * localnamescope = new LayoutType("__f");
    AstNode * name = children[0];
    AstNode * args = children[1];
    AstNode * ret = children[2];
    
    ret->inferTypes();
    if(!ret->val_type){
        LayoutType * temp_ns = LayoutType::createNamespace();
        Type * ret_t = (*temp_ns)[ret->memval];
        if(ret_t){
            ret->val_type = ret_t;
        } else{
            ret_t = &VoidType;
        }
    }
    Type * ret_type  = &VoidType;
    if(ret->val_type != &VoidType){
        ret_type = (*(LayoutType*)ret->val_type)["return"];
    }
    args->setNameScope(LayoutType::createNamespace(), 0);
    args->inferTypes();
    LayoutType * functype = LayoutType::createFunctionalType(ret_type, args->val_type);
    
    val_type = functype;
    name->val_type = functype;
    namescope->addType(name->memval, functype);
    
    AstNode * body = children[3];
    if(children.size() >= 4){
        body->setNameScope(localnamescope, os);
        if(args->t == TUPLE){
            for(auto c : args->children){
                //must be typedecl, already type inferred
                localnamescope->addType(c->children[0]->memval, c->children[0]->val_type);
                
                
            }
        } else if (args->t == TYPEDECL){
            localnamescope->addType(args->children[0]->memval, args->children[0]->val_type);
        } else if (args->t != EMPTY){
#warning TODO: semantic error
        }
        body->inferTypes();
    }
}



void AstNode::inferClassDeclTypes(){
    AstNode * classname = children[0];
    AstNode * body = children[1];
    LayoutType * classLayout = new LayoutType(classname->memval);
    LayoutType * vtable = new LayoutType("vtable");
    classLayout->addType("vtable", vtable);
    for(auto member: body->children){
        if(member->t == TYPEDECL || member->t == FUNTYPEDECL){
            
            
            AstNode * member_name = member->children[0];
            AstNode * rhs = member->children[1];
            if(rhs->t == SUBSCR){
                //is mapping
                Type * valtype = (*(LayoutType*)(rhs->children[0]->val_type))["return"];
                Type * mapType;
                if(rhs->children[1]->t != EMPTY)
                    mapType = (*(LayoutType*)(rhs->children[1])->val_type)["return"];
                else
                    mapType = &IntType;
                LayoutType * newMapType = LayoutType::createMap(mapType, valtype);
                rhs->val_type = newMapType;
                member->children[0]->val_type = newMapType;
            } else if(rhs->t == NAME){
                Type * local_return = (*namescope)[rhs->memval];
                if(!local_return){
                    LayoutType * temp_ns = LayoutType::createNamespace();
                    member->children[0]->val_type = (*(LayoutType*)(temp_ns->operator[](member->children[1]->memval)))["return"];
                    delete temp_ns;
                } else {
                    member->children[0]->val_type = (*(LayoutType*)local_return)["return"];
                }
                
                
            } else {
                children[0]->val_type = children[1]->val_type;
            }
            classLayout->addType(member_name->memval, member_name->val_type);
        }
        if(member->t == FUNCDEF){
#warning TODO method decls
            member->parseMethodDecl(classLayout);
            vtable->addType(member->children[0]->memval, member->val_type);
            
        }
        
    }
    
    val_type = LayoutType::createFunctionalType(classLayout, &VoidType);
    namescope->addType(classname->memval, val_type);
}


void AstNode::inferTypes(){
    if(val_type)
        return;
    if(namescope == nullptr){
        throw "cannot infer types without context";
    }
    
    if(t == CLASSDEF){
        inferClassDeclTypes();
        return;
    }
    
    
    if(t != ASSIGN && t!= FUNCDEF)
    for(int i = 0; i < children.size(); i++){
        children[i]->inferTypes();
    } else children[1]->inferTypes();
    if(t == FUNCDEF){
        //children[3]->inferTypes();
    }
    switch (t) {
        case EMPTY:
            val_type = &VoidType;
            break;
        case INTLIT:
            val_type = &IntType;
            break;
        case FLOATLIT:
            val_type = &FloatType;
            break;
        case STRLIT:
            val_type = &StringType;
            break;
        case TYPECAST:{
            Type * _t;
            if(children[1]->t == NAME)
                 _t = (*(LayoutType*)children[1]->val_type)["return"];
            else{
                _t = children[1]->val_type;
            }
            val_type = _t;
        }break;
        case FCALL:{
            val_type = (*(LayoutType*)children[0]->val_type)["return"];
        }break;
        case FUNTYPEDECL:{
            Type * arg=0, *ret;
            if(children[0]->t == TUPLE){
                std::vector<Type*> types;
                for(auto c: children[0]->children){
                    types.push_back((*(LayoutType*)c->val_type)["return"]);
                }
                arg = LayoutType::createTuple(types);
            } else if(children[0]->t == NAME){
                arg = (*(LayoutType*)children[0]->val_type)["return"];
            } else if(children[0]->t == EMPTY){
                arg = &VoidType;
            } else {
#warning TODO: throw semantic error
            }
            ret = (*(LayoutType*)children[1]->val_type)["return"];
            val_type = LayoutType::createFunctionalType(ret, arg);
        }break;
        case INTERFACE:
        case CLASSDEF:{
            AstNode * classname = children[0];
            AstNode * body = children[1];
            LayoutType * classLayout = new LayoutType(classname->memval);
            for(auto member: body->children){
                if(member->t == TYPEDECL || member->t == FUNTYPEDECL){
                    
                    
                    AstNode * member_name = member->children[0];
                    classLayout->addType(member_name->memval, member_name->val_type);
                }
                if(member->t == FUNCDEF){
#warning TODO method decls
                }
                
            }
            
            val_type = LayoutType::createFunctionalType(classLayout, &VoidType);
            namescope->addType(classname->memval, val_type);
        }break;
        case MUL:
        case ADD:
        case SUB:
        case DIV:{
            AstNode * lhs = children[0];
            AstNode * rhs = children[1];
            if(lhs->val_type == &IntType && rhs->val_type == &IntType){
                val_type = &IntType;
            } else if( lhs->val_type->isNumeric() && rhs->val_type->isNumeric() ){
                //both are numeric but not both are integer
                val_type = &FloatType;
                AstNode * fltype = new AstNode(NAME);
                fltype->setNameScope(namescope, outerscope);
                fltype->val_type = &FloatType;
                
                
                
                if(lhs->val_type == &IntType){
                    children[0] = new AstNode(TYPECAST, {lhs, fltype});
                    children[0]->val_type = &FloatType;
                }
                if(rhs->val_type == &IntType){
                    children[1] = new AstNode(TYPECAST, {rhs, fltype});
                    children[1]->val_type = &FloatType;
                }
                
            }
            if(lhs->val_type == &StringType && rhs->val_type == &StringType && t == ADD){
                val_type = &StringType;
            }
            if(!val_type)
                throw new SyntaxError{0, 0, "cannot infer type"};
            
        }break;
        case RET:
        case WRITE:
            val_type = children[0]->val_type;
            break;
        case AT:{
            LayoutType * parental_type = (LayoutType*)children[0]->val_type;
			if(parental_type == nullptr){
				throw SyntaxError{0, 0, "parental type at subscr is NULL"};
			}
            const char * member_name = children[1]->memval;
            val_type = (*parental_type)[member_name];
            if(!val_type){
				LayoutType * vtable = (LayoutType*)(*parental_type)["vtable"];
				if(!vtable){
						
				} else {
					val_type = (*vtable)[member_name];
				}
			}
            
        }break;
        case NAME:{
            val_type = (*namescope)[memval];
            if(!val_type){
                AstNode * walker = this;
                while(walker && walker->t != FUNCDEF){
                    walker = walker->parent;
                }
                
                
                
                
                walker;
            }
        }break;
        case TYPEDECL:{
            AstNode * rhs = children[1];
            if(rhs->t == SUBSCR){
                //is mapping
                Type * valtype;
                if(rhs->children[0]->t == NAME)
                 valtype = (*(LayoutType*)(rhs->children[0]->val_type))["return"];
                else
                    valtype = rhs->children[0]->val_type;
                Type * mapType;
                if(rhs->children[1]->t == NAME)
                    mapType = (*(LayoutType*)(rhs->children[1])->val_type)["return"];
                else
                    mapType = &IntType;
                LayoutType * newMapType = LayoutType::createMap(mapType, valtype);
                rhs->val_type = newMapType;
                children[0]->val_type = newMapType;
            } else if(rhs->t == NAME){
                Type * local_return = (*namescope)[rhs->memval];
                if(!local_return){
                    LayoutType * temp_ns = LayoutType::createNamespace();
                    children[0]->val_type = (*(LayoutType*)(temp_ns->operator[](children[1]->memval)))["return"];
                    delete temp_ns;
                } else {
                    children[0]->val_type = (*(LayoutType*)local_return)["return"];
                }
                
                
            } else {
                children[0]->val_type = children[1]->val_type;
            }
            namescope->addType(children[0]->memval, children[0]->val_type);
            val_type = children[0]->val_type;
        }break;
        case TUPLE:{
            std::vector<Type*> v;
            for(const auto child:children){
                v.push_back(child->val_type);
            }
            val_type = LayoutType::createTuple(v);
        }break;
            
        case FUNCDEF:{
            parseFundecl(outerscope);
            
        }break;
            
            
        case EQUALS:
        case NEQ:
        case GEQ:
        case LEQ:
        case LESS:
        case GREATER:
            
#warning TODO: check argument types for comparability
            
            val_type = &BoolType;
            break;
        case _FALSE:
        case _TRUE:
            val_type = &BoolType;
            break;
        case ASSIGN:{
            if(children[0]->t == TUPLE){
#warning TODO: tuple case
            } else if(children[0]->t == NAME){
#warning TODO: subscr case
                children[0]->val_type = children[1]->val_type;
                Type * hadtype = (*namescope)[children[0]->memval];
                if(hadtype && hadtype != children[0]->val_type){
                    throw TypeError{hadtype, val_type};
                }
                if(!hadtype){
                    namescope->addType(children[0]->memval, children[0]->val_type);
                }
                
            } else if(children[0]->t == AT){
                children[0]->inferTypes();
            }
        }break;
        case SUBSCR:{
#warning TODO: typecheck for key
            val_type = (*(LayoutType*)children[0]->val_type)["value"];
        }break;
            
        default:
            break;
    }
}

template<typename T>
T performStatic(T lhs, T rhs, AstNode::type t){
    switch (t) {
        case AstNode::ADD:
            return lhs + rhs;
            break;
        case AstNode::SUB:
            return lhs - rhs;
            break;
        case AstNode::MUL:
            return lhs * rhs;
            break;
        case AstNode::DIV:
            return lhs / rhs;
            break;
        default:
            return T();
            break;
    }
}


void AstNode::removeRedundant(){
    for(auto c: children){
        c->removeRedundant();
    }
    if(t == TYPECAST && children[0]->isNumLiteral()){
        if(val_type == &FloatType){
            double val;
            if(children[0]->t == INTLIT){
                val = *(int64_t*)children[0]->memval;
            } else {
                val = *(double*)children[0]->memval;
            }
            memcpy(memval, &val, 8);
            t = FLOATLIT;
        }
        if(val_type == &IntType){
            int64_t val;
            if(children[0]->t == INTLIT){
                val = *(int64_t*)children[0]->memval;
            } else {
                val = *(double*)children[0]->memval;
            }
            memcpy(memval, &val, 8);
            t = INTLIT;
        }
        children.clear();
    }
    if((t == ADD || t == SUB || t == MUL || t == DIV) && children[0]->isNumLiteral() && children[1]->isNumLiteral()){
        // as they are properly type casted, we can just perform the operation
        if(val_type == &IntType){
            int64_t lhs = *(int64_t*)children[0]->memval, rhs= *(int64_t*)children[1]->memval;
            *(int64_t*)memval = performStatic(lhs, rhs, t);
            t = INTLIT;
        } else{
            double lhs = *(double*)children[0]->memval, rhs= *(double*)children[1]->memval;
            *(double*)memval = performStatic(lhs, rhs, t);
            t = FLOATLIT;
        }
        children.clear();
        
    }
}

void AstNode::inferTypesAsObject(){
    val_type = &ObjectType;
    if(t == FLOATLIT)
        val_type = &FloatType;
    if(t == INTLIT)
        val_type = &IntType;
    if(t == NAME){
        if(namescope->indexOf(memval) == -1){
            namescope->addType(memval, &ObjectType);
        }
    }
    for(auto c: children){
        c->inferTypesAsObject();
    }
}
