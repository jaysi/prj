#include "double.h"

static inline int _run_if(struct session* sess){
	
    struct tok_s* block, *logic;
    int ret;
    DEF_LOAD_RES_VARS();

	block = poptok(sess);//this is code block
	if( !block ){
        printe(MSG_BADUSE, "if");
		return false_;
	}

    logic = poptok(sess);
    if( !logic ){
        printe(MSG_BADUSE, "if");
        return false_;
    }

    if(!tokval(sess, logic)){
        printd13("logic invalid\n");
        sess->cursess->pre->exp->ifcond = IFCOND_FALSE;
        //restore_prefix(sess);
        return true_;
    }

    LOAD(1);
    
    //the RESTORE, restores this too!
    sess->cursess->pre->exp->ifcond = IFCOND_TRUE;    
    
    if(!sess->cursess->pre->exp->branch){//not precompiled
        if(!_precompile(sess, block->str, &sess->cursess->pre->exp->branch)){
            RESTORE(1);
            return false_;
        }
        sess->cursess->pre = sess->cursess->pre->exp->branch;
    } else {
        sess->cursess->pre = sess->cursess->pre->exp->branch;
        if(!restore_prefix(sess)){
            RESTORE(1);
            return false_;
        }
    }

    //the problem IS: the jmp runs inside the branch,
    //so the restore prefix cannot operate well
    ret = run_ci(sess);
    
    RESTORE(1);
        
    return ret;
}

int _else(struct session* sess){
    struct tok_s *block;
    int ret = true_;
    DEF_LOAD_RES_VARS();
    
    printd13("ifcond = %s\n", sess->cursess->pre->exp->ifcond == IFCOND_FALSE?"false":sess->cursess->pre->exp->ifcond == IFCOND_TRUE?"true":"neut");
	
	if(sess->cursess->pre->exp->ifcond != IFCOND_FALSE && sess->cursess->pre->exp->ifcond != IFCOND_TRUE){
        printe(MSG_BADUSE, "else");
		return false_;
	}	
	
	printd13("got else\n");
	block = poptok(sess);//this is code block
	if( !block ){
        printe(MSG_BADUSE, "else");
		return false_;
	}
	
    if(sess->cursess->pre->exp->ifcond == IFCOND_FALSE){
	
		printd13("running tok \'%s\'\n", block->str);
	
		//run body
		
//		if(!_run_codeblock(sess, block->str)){
//			sess->cursess->pre->exp->ifcond = IFCOND_ENDIF;
//			return false_;
//		}

        LOAD(1);

        if(!sess->cursess->pre->exp->branch){//not precompiled
            if(!_precompile(sess, block->str, &sess->cursess->pre->exp->branch)){
                RESTORE(1);
                return false_;
            }
            sess->cursess->pre = sess->cursess->pre->exp->branch;
        } else {
            sess->cursess->pre = sess->cursess->pre->exp->branch;
            if(!restore_prefix(sess)){
                RESTORE(1);
                sess->cursess->pre->exp->ifcond = IFCOND_ENDIF;
                return false_;
            }
        }

        ret = run_ci(sess);

        RESTORE(1);

	}
	
	sess->cursess->pre->exp->ifcond = IFCOND_ENDIF;
	
	return ret;
	
}

int _if(struct session* sess){
	
	if(sess->cursess->pre->exp->ifcond != IFCOND_NOIF){
        printe(MSG_FORBID, "if");
		return false_;
	}
		
	return _run_if(sess);
	
}

int _else_if(struct session* sess){

	if(sess->cursess->pre->exp->ifcond != IFCOND_FALSE && sess->cursess->pre->exp->ifcond != IFCOND_TRUE){
        printe(MSG_BADUSE, "elseif");
		return false_;
	}
	
	if(sess->cursess->pre->exp->ifcond == IFCOND_TRUE){
		poptok(sess);
		poptok(sess);
		return true_;
	}
	
	return _run_if(sess);
}


