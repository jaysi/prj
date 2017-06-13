#include "double.h"

//const char delimlist[] = "(){}[]+-=/^% \t\"\'";
//const char packlist[] = "\"\'";
//const char escape = '\\';

char* gettok(   char* exp,//expression
                const char* dlist,//delim list
                const char esc,//escape character
                const char* plist,//must be defined inside the delimlist too
                const char* blist,//like above, blist[0]=bstart, blist[1]=bend
                char* buf,//buffer to put token
                size_t* bufsize,//buffersize
                char* delim//delim caused interrupt
                ){
    char packchar = 0;
    int bdepth = 0;
    char* end = exp;
    
    assert(exp);
    
    //printd10("expression: %s\n", exp);

resume:
    if(!packchar && !bdepth){
    	end = strpbrk(end, dlist);
    } else {
    	
		if(plist && !bdepth){
    		end = strchr(end, packchar);
    	}
    	
    	if(blist && !packchar){
    		end = strchr(end, blist[1]);
    	}
    	
    }
//    else if(blist && bdepth) end = strchr(end, blist[1]);
//    else if(plist) end = strchr(end, packchar);
 
    //handle end of expression
    if(!end){
    	
    	if(bdepth > 1){//block left open!
    		*bufsize = 0;
    		*delim = blist[0];
    		return NULL;
    	}

        if(packchar){//string token left open!
            *bufsize = 0;
            *delim = packchar;
            return NULL;
        }

        if(strlen(exp) < *bufsize){//ok, expression ends normally
            *bufsize = strlen(exp);
            memcpy(buf, exp, *bufsize);
            buf[*bufsize] = '\0';
            *delim = '\0';
        } else {//not enough token buffer
            *bufsize = 0;
        }

        return NULL;
    }

    //handle esc char
    if(end != exp && *(end-1) == esc){
        strcpy(end - 1, end);
        end++;
        goto resume;//continue
    }

	if(plist && !bdepth){
		if(!packchar){
	        if(strchr(plist, *end)){//delim == pack_char_list
	            packchar = *end;
	            end++;
	            goto resume;//continue
	        }			
		} else {
	        if(*end != packchar){//continue
	            end++;
	            goto resume;
	        }  else {//ignore first packchar
	            if(*exp == packchar) exp++;//remember the case round(1.123,2,"trim");
	            //packchar = 0;
	            //the next char should be a delimiter too? not for now!
	        }
		}
	}

	if(blist && !packchar){
		if(!bdepth){
			
			if(*end == blist[0]){
				bdepth++;				
				end++;
				goto resume;
			}
			//if(*end == blist[1]) return NULL;//?
			
		} else {

	        if(*end != blist[1]){//continue
	        
	        	if(*end == blist[0]) bdepth++;
	        	
	            end++;
	            goto resume;
	        }  else {//*end = }
	        	bdepth--;
	        	if(bdepth){
	        		end++;
	        		goto resume;
	        	}
	        	if(strchr(dlist, *(end+1))) end++;//to handle if(){}else{}
	        }
			
		}
	}
	    
    if((unsigned)(end - exp) > *bufsize){//not enough token buffer
        *bufsize = 0;
        return NULL;
    }

    //everything's ok, return
    *delim = *end;
    *bufsize = end - exp;    
    memcpy(buf, exp, *bufsize);
    buf[*bufsize] = '\0';    
    return (end+1);
}

int push_infix(struct session* sess){
	            
	sess->cursess->pre->exp->prevtok = NULL;
            
	for(sess->cursess->pre->exp->curtok = sess->cursess->pre->exp->infixfirst;
		sess->cursess->pre->exp->curtok;
		sess->cursess->pre->exp->curtok = sess->cursess->pre->exp->curtok->infixnext){
            
	    switch(sess->cursess->pre->exp->curtok->type){
	    	
	        case TOKT_COMMAND:
	        case TOKT_MATH_FUNC:
	        case TOKT_FUNC:
			case TOKT_MATH_LOGIC:			
	        case TOKT_BLOCK://should i push the code blocks? yep!	        	
	        push(sess);
	        break;
	
	        case TOKT_MATH_BASIC:	        		        
	        
	        switch(sess->cursess->pre->exp->curtok->id){
	
	            case TOK_ASSIGN:					            
	            /*
	             * variable assignment, copy the rest to the variable,
	             * the variable must be at prev_tok and at prefix_first
	             * unless there is a help request
	            */
	            	//this is a variable definition request, save the position of the curtok for further use.
//            	if(!sess->cursess->pre->exp->vdeftok){
//            		sess->cursess->pre->exp->vdeftok = sess->cursess->pre->exp->curtok->infixnext;
//					sess->cursess->pre->exp->varinfix = sess->cursess->pre->exp->infixrest;
//					printd5("set infix rest to: %s\n", sess->cursess->pre->exp->varinfix);
//            	} else {
//            		printe(MSG_SYNTAX2, "variable", "assignment");
//            		return false_;
//            	}
	            
				push(sess);
	
	            break;
	
	            case TOK_BR_CLOSE:
	            if(!pop_to_br(sess)){
	                /*
	                printe("there was no matching brackets near \"%s %s\"."PARAEND,
	                        sess->cursess->pre->exp->prevtok?sess->cursess->pre->exp->prevtok->str:"",
	                        sess->cursess->pre->exp->curtok->str);
	                */
	                if(!(sess->cursess->pre->flags & PREF_FOR_ADVANCE)){
	                    printe(MSG_MATCH,
	                           sess->cursess->pre->exp->prevtok?sess->cursess->pre->exp->prevtok->str:"",
	                           sess->cursess->pre->exp->curtok->str);
	                    return false_;
	                } else {
	                    //ignore this error once for the "for advance expression"
	                    sess->cursess->pre->exp->flags &= ~PREF_FOR_ADVANCE;
	                }
	            }
	            break;
	
	            case TOK_ABR_CLOSE:						            
	            if(!pop_to_abr(sess)){
	                /*
	                printe("there was no matching array brackets near \"%s %s\"."PARAEND,
	                        sess->cursess->pre->exp->prevtok?sess->cursess->pre->exp->prevtok->str:"",
	                        sess->cursess->pre->exp->curtok->str);
	                */
	                printe(MSG_MATCH,
	                       sess->cursess->pre->exp->prevtok?sess->cursess->pre->exp->prevtok->str:"",
	                       sess->cursess->pre->exp->curtok->str);
	                return false_;
	            }
	            sess->cursess->calc = sess->cursess->calcreg;
	            break;
	            
	            case TOK_ABR_OPEN:
	            	sess->cursess->calcreg = sess->cursess->calc;
	
	            default:
	            
	            push(sess);
	            
	            break;
	        }
	
	        break;
	
	        case TOKT_SPEC:
	        	
	            switch(sess->cursess->pre->exp->curtok->id){
		            case TOK_COL://ignore?
		            
		            //printw("got ,\n");
		            
		            if(!pop_to_br_for_col(sess)){
	                    /*
	                    printe("there was no matching brackets or columns near \"%s %s\"."PARAEND,
	                            sess->cursess->pre->exp->prevtok?sess->cursess->pre->exp->prevtok->str:"",
	                            sess->cursess->pre->exp->curtok->str);
	                    */
	                    printe(MSG_MATCH,
	                           sess->cursess->pre->exp->prevtok?sess->cursess->pre->exp->prevtok->str:"",
	                           sess->cursess->pre->exp->curtok->str);
		                return false_;	            	
		            }
		            
		            push(sess);
		            
		            break;
		            
		            default:
		            break;
		        }   
				     	
	        break;
				        	        
	        default://number? variable?
	        	
	        insert_pre(sess, sess->cursess->pre->exp->curtok);
	        
	        break;
	    }	
    }
    
    return true_;

}

int proc_tok(struct session* sess, char* tokbuf, enum tok_t delim){    

    //resolve element

    sess->cursess->pre->exp->curtok = make_stacktok(sess, tokbuf);
    sess->cursess->pre->exp->curtok->infixnext = NULL;
    sess->cursess->pre->exp->curtok->next = NULL;
    if(!sess->cursess->pre->exp->curtok){
        printe(MSG_OVERFLOW, "stack");
        return false_;
    }

	switch(delim){
		case TOK_STRPACK://enforcement is not pretty, again!
			morph_tok(sess, sess->cursess->pre->exp->curtok, TOK_STR);
//            sess->cursess->pre->exp->curtok->id = TOK_STR;
//            sess->cursess->pre->exp->curtok->type = TOKT_STRING;
            
		break;
		
		case TOK_ASSIGN:
			
        	if(!sess->cursess->pre->exp->vdeftok){
        		sess->cursess->pre->exp->vdeftok = sess->cursess->pre->exp->curtok->infixnext;
				sess->cursess->pre->exp->varinfix = sess->cursess->pre->exp->infixrest;
//				sess->cursess->pre->exp->ansreg = var_val(sess, ANSVARNAME);
				printd5("set infix rest to: %s\n", sess->cursess->pre->exp->varinfix);
        	} else {
        		printe(MSG_SYNTAX2, "variable", "assignment");
        		return false_;
        	}			
			
			break;
		
		default:
			break;

	}

    printd("got curtok, %s, %i, next: %s.\n", sess->cursess->pre->exp->curtok->str, sess->cursess->pre->exp->curtok->id, sess->cursess->next);    
    
    if(!sess->cursess->pre->exp->infixfirst){
    	   
    	sess->cursess->pre->exp->infixfirst = sess->cursess->pre->exp->curtok;
    	sess->cursess->pre->exp->infixlast = sess->cursess->pre->exp->curtok;

		printd("set infix first to %s\n", sess->cursess->pre->exp->infixfirst->str);
    	
    } else {
    	sess->cursess->pre->exp->infixlast->next = sess->cursess->pre->exp->curtok;
    	sess->cursess->pre->exp->infixlast->infixnext = sess->cursess->pre->exp->curtok;
    	sess->cursess->pre->exp->infixlast = sess->cursess->pre->exp->curtok;
    	printd("set infix last to %s\n", sess->cursess->pre->exp->infixlast->str);
    }

    return continue_;
}

int tokenize(struct session* sess){

    size_t toksize;
    char* tmp;
    enum tok_t delimtok;

    toksize = sess->cursess->tokbufsize;

    //to handle ) at the end of expression
    if(!sess->cursess->pre->exp->infixrest[0]) return true_;
    
    tmp = gettok(   sess->cursess->pre->exp->infixrest,
                    dbl_delimlist,
					dbl_escape,
					dbl_packlist,
					dbl_blocklist,
                    sess->cursess->tokbuf,
					&toksize,
                    sess->cursess->pre->exp->delim);

    if(!tmp){//analyze NULL return

        if(!toksize){//error, what to do when ) is at the end?
            /*
            printe( "there was an error parsing token \"%s\" before delimiter \"%s\""PARAEND,
                    sess->cursess->pre->exp->infixrest, sess->cursess->pre->exp->delim);
            */
            printe(MSG_PARSE2, sess->cursess->pre->exp->infixrest, sess->cursess->pre->exp->delim);
            return false_;
        }

    }

    //set the rest here to make proc_tok() able to extract var assignments
    sess->cursess->pre->exp->infixrest = tmp;
    printd("the infix rest is, %s\n", sess->cursess->pre->exp->infixrest);

    printd("got token \'%s\', delim \'%s\'"PARAEND, sess->cursess->tokbuf, sess->cursess->pre->exp->delim);

    delimtok = tokid(sess, sess->cursess->pre->exp->delim);

    if(toksize){
        switch(proc_tok(sess, sess->cursess->tokbuf, delimtok)){
        	case false_://error
                sess->cursess->pre->flags &= ~PREF_FOR_ADVANCE;
            	return false_;
            	break;
            case true_://return, (if) used?
            	printd9("if returned ok?");
            	return true_;
            	break;
			default://continue
				break;
        }
    }

    if(sess->cursess->pre->exp->delim[0]){
		switch(delimtok){
			case TOK_WHITE:
			case TOK_STRPACK:
			case TOK_BLOCK_CLOSE:
				break;
			default:
                if(!proc_tok(sess, sess->cursess->pre->exp->delim, TOK_INVAL)){
		            return false_;
		        }
				break;
		}
	} else {
        printd("end of expression\n");
        return true_;
    }

    //ok and continue
    return continue_;
}

int prefix(struct session* sess){

	struct tok_s* tk;

/*
	in some cases, e.g. mode; the functions does not push inside the prefix
*/

    sess->cursess->pre->exp->infixrest = sess->cursess->pre->exp->infix;
    
    init_stack(sess);

go_on:
	
	_show_stack(sess);
	_show_prefix(sess);	
	
    switch(tokenize(sess)){

        case false_:
            return false_;
        break;

        case true_://expression ends, just push token
            printd("expression ends, returning.\n");            
        break;
        case continue_:
            goto go_on;
        break;
    }
    
    if(!combine(sess)){
    	return false_;
    }
    
    if(!push_infix(sess)){
    	return false_;
    }
    
    //now flush everything to the prefix;
    while((tk = poptok(sess))){
    	insert_pre(sess, tk);
    }
        
    sess->cursess->pre->exp->saveprefixfirst = sess->cursess->pre->exp->prefixfirst;
    
    //TODO: load state CRASHes HERE!
    //assert(sess->cursess->pre->exp->saveprefixfirst);
    //printd("saved %s as prefix first"PARAEND, sess->cursess->pre->exp->saveprefixfirst->str);

    return true_;
}

int restore_prefix(struct session* sess){

    struct tok_s* ent;
    struct pre_exp* cur;
    
    if(!sess->cursess->pre || !sess->cursess->pre->expfirst) return continue_;

    for(sess->cursess->pre->exp = sess->cursess->pre->expfirst;
		sess->cursess->pre->exp;
		sess->cursess->pre->exp = sess->cursess->pre->exp->next){
		
		assert(sess->cursess->pre->exp->saveprefixfirst);
		
	    sess->cursess->pre->exp->prefixfirst =
			sess->cursess->pre->exp->saveprefixfirst;
			
		printd("save prefix first: %s"PARAEND, sess->cursess->pre->exp->saveprefixfirst->str);
	
		//strcpy(sess->cursess->outbuf, "### PREFIX: ");
	
	    for(ent = sess->cursess->pre->exp->prefixfirst;
			ent;
			ent = ent->prenext){
	    	//TODO: this is a temporary work-around!
            //cant use ent->id == TOK_INT, see vstrcmp
	    	if(	ent->type == TOKT_NUMBER || ent->id == TOK_VAR ||
				ent->id == TOK_ARRAY){
                if(ent->id == TOK_INT || ent->id == TOK_REAL || ent->id == TOK_SCIENCE){

                    //TODO: is this a BUG? (TOKF_NEG???)
                    if(ent->valreg < 0){
                        ent->val = -ent->valreg;
                        ent->flags |= TOKF_NEG;
                    } else {
                        ent->val = ent->valreg;
                        ent->flags &= ~TOKF_NEG;
                    }

                } else {
                    ent->flags &= ~TOKF_RES;
                    //TODO: only i = -1 is enough?
                    ent->i = -1;//enforce re-assignment to avoid the x[2] -> x[2][2] bug!
                    ent->j = -1;
                }
	    	}
	    	
			//snprintf(sess->outbuf + strlen(sess->outbuf), sess->outbufsize - strlen(sess->outbuf), "%s ", ent->str);
			
	        ent->next = ent->prenext;
	    }
	    
	    //strcat(sess->outbuf, PARAEND);
	    
		//textout_(sess->cursess->outbuf);
		
		//restore branches too!
		if(sess->cursess->pre->exp->branch){
			printd("restoring branch."PARAEND);
			cur = sess->cursess->pre;
			
			sess->cursess->pre = sess->cursess->pre->exp->branch;
			
			restore_prefix(sess);
			
			sess->cursess->pre = cur;
		}
    }

    //_show_pre(sess, sess->cursess->pre);
	
    return true_;
}
