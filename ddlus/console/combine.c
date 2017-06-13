#include "double.h"

//#define FLAG_TEST_NEG	(0x0001<<0)

struct iivcombi_s {
	enum tok_t pre;
	enum tok_t cur;
	enum tok_t com;
} iivcombi[] = {
	{TOK_PLUS, TOK_ASSIGN, TOK_PLUS_EQ},
	{TOK_MINUS, TOK_ASSIGN, TOK_MINUS_EQ},
	{TOK_AND, TOK_AND, TOK_AND_AND},
	{TOK_OR, TOK_OR, TOK_OR_OR},
	{TOK_ASSIGN, TOK_ASSIGN, TOK_EQ},
	{TOK_LT, TOK_ASSIGN, TOK_LE},
	{TOK_GT, TOK_ASSIGN, TOK_GE},
	{TOK_NOT, TOK_ASSIGN, TOK_NE},
	{TOK_MINUS, TOK_MINUS, TOK_MINUS_MINUS},
	{TOK_PLUS, TOK_PLUS, TOK_PLUS_PLUS},
	{TOK_INVAL, TOK_INVAL, TOK_INVAL}
};

int morph_tok(struct session* sess, struct tok_s* tk, enum tok_t dst_id) {
	
	struct tok_s* dst_tok;	
	
	dst_tok = tokidptr(sess, dst_id);
	
	if(!dst_tok) return false_;
	
	printd5("morph %s(%i) to %s(%i)\n", tk->str, tk->id, dst_tok->form[0], dst_tok->id);
	
	tk->id = dst_id;
	tk->type = dst_tok->type;
	tk->pred = dst_tok->pred;	
	
	if(dst_id != TOK_STR) tk->str = dst_tok->form[0];
	
	return true_;
	
}

enum tok_t find_iicombi(struct session* sess){	

	struct iivcombi_s* s;
	
	if(!sess->cursess->pre->exp->curtok->infixnext) return TOK_INVAL;
	
	for(s = iivcombi; s->pre != TOK_INVAL; s++){
		
		printd_comb1("prev: %i, curr: %i, cprev: %i, ccurr: %i\n", sess->cursess->pre->exp->prevtok?sess->cursess->pre->exp->prevtok->id:0, sess->cursess->pre->exp->curtok->id, s->pre, s->cur);
		
		if( s->pre == sess->cursess->pre->exp->curtok->id &&
			s->cur == sess->cursess->pre->exp->curtok->infixnext->id)
				return s->com;
	}
	
	return TOK_INVAL;
}

int combine(struct session* sess){
	
	enum tok_t comb;	
//	unsigned short flags = 0;
	
	assert(sess->cursess->pre->exp->infixfirst);
	
	sess->cursess->pre->exp->prevtok = NULL;
	for(sess->cursess->pre->exp->curtok = sess->cursess->pre->exp->infixfirst;
		sess->cursess->pre->exp->curtok;
		sess->cursess->pre->exp->curtok = sess->cursess->pre->exp->curtok->infixnext){
			
		printd_comb1("current tok: %s\n", sess->cursess->pre->exp->curtok->str);
		printd_comb1("prev tok: %s\n", sess->cursess->pre->exp->prevtok?sess->cursess->pre->exp->prevtok->str:"NULL");				
		
//		if(flags & FLAG_NEGATIVE_NEXT){
			
//			sess->cursess->pre->exp->curtok->flags |= TOKF_NEG;
//			//sess->cursess->pre->exp->curtok->flags &= ~TOKF_RES;
//			flags &= ~FLAG_NEGATIVE_NEXT;
			
//		}
		
//		if(sess->cursess->pre->exp->prevtok)
//		switch( sess->cursess->pre->exp->curtok->id ){
			
//			case TOK_MINUS:

//			switch( sess->cursess->pre->exp->prevtok->type ){
				
//				case TOKT_MATH_BASIC:
//				case TOKT_MATH_LOGIC:
					
//					//handle var++ or var-- cases;
//					if(sess->cursess->pre->exp->curtok->infixnext){
//						if(flags & FLAG_NEGATIVE_NEXT){
//							printe(MSG_SYNTAX2, sess->cursess->pre->exp->prevtok->str, sess->cursess->pre->exp->curtok->str);
//							return false_;
//						} else {
//							flags |= FLAG_NEGATIVE_NEXT;
//						}
//					}
				
//				break;
				
//				default:
//					break;
//			}
			
//			break;
			
//			default:
//				break;
		
//		}

		comb = find_iicombi(sess);

		if(comb != TOK_INVAL){

			printd_comb1("found combination.\n");						
			
			morph_tok(sess, sess->cursess->pre->exp->curtok->infixnext, comb);

			//jmp
//			sess->cursess->pre->exp->curtok = sess->cursess->pre->exp->curtok->infixnext;
//			sess->cursess->pre->exp->curtok = sess->cursess->pre->exp->curtok->next;

			if(sess->cursess->pre->exp->prevtok){
				sess->cursess->pre->exp->prevtok->next = sess->cursess->pre->exp->curtok?sess->cursess->pre->exp->curtok->next:NULL;
				sess->cursess->pre->exp->prevtok->infixnext = sess->cursess->pre->exp->curtok?sess->cursess->pre->exp->curtok->infixnext:NULL;
			} else {
				printd16("the sess->cursess->pre->exp->prevtok = NULL\n");
			}

		    //check infix syntax
			if(!syntax(sess)){
		        printe(MSG_SYNTAX2, sess->cursess->pre->exp->prevtok?sess->cursess->pre->exp->prevtok->str:"", sess->cursess->pre->exp->delim);
		        return false_;
		    }		    		    

		} else {

            if(sess->cursess->pre->exp->curtok->id == TOK_MINUS){

                if( sess->cursess->pre->exp->prevtok &&
                    ((	sess->cursess->pre->exp->prevtok->type == TOKT_MATH_BASIC &&
						sess->cursess->pre->exp->prevtok->id != TOK_BR_CLOSE &&
						sess->cursess->pre->exp->prevtok->id != TOK_ABR_CLOSE) ||//+
                     sess->cursess->pre->exp->prevtok->type == TOKT_MATH_LOGIC ||//&
                     sess->cursess->pre->exp->prevtok->type == TOKT_MATH_FUNC ||//sin
                     sess->cursess->pre->exp->prevtok->id == TOK_COL //,
                     )){

                        if( sess->cursess->pre->exp->curtok->infixnext &&
                            (sess->cursess->pre->exp->curtok->infixnext->type == TOKT_NUMBER ||
                             sess->cursess->pre->exp->curtok->infixnext->id == TOK_VAR ||
                             sess->cursess->pre->exp->curtok->infixnext->id == TOK_ARRAY)){

                            sess->cursess->pre->exp->curtok->infixnext->flags |= TOKF_NEG;
                            
                            //jump over it!
                            sess->cursess->pre->exp->prevtok->infixnext = sess->cursess->pre->exp->curtok->infixnext;
                            sess->cursess->pre->exp->prevtok->next = sess->cursess->pre->exp->curtok->next;
                            sess->cursess->pre->exp->curtok = sess->cursess->pre->exp->curtok->infixnext;

                        } else {
                            printe(MSG_SYNTAX2, sess->cursess->pre->exp->curtok->infixnext->str, sess->cursess->pre->exp->delim);
                            return false_;
                        }
                    }
            }

		    //check infix syntax
			if(!syntax(sess)){
		        printe(MSG_SYNTAX2, sess->cursess->pre->exp->prevtok?sess->cursess->pre->exp->prevtok->str:"", sess->cursess->pre->exp->delim);
		        return false_;
		    }
		    			
			sess->cursess->pre->exp->prevtok = sess->cursess->pre->exp->curtok;
		}
	}
	
	return true_;
	
}
