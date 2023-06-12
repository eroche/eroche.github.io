// wcomp_runtime.c

#include"lang.h"



#pragma mark - WCOMP_MEM

WCOMP_MEM init_wcomp_mem(void){
    WCOMP_MEM wmem=(WCOMP_MEM)L_ALLOC(sizeof(struct _wcomp_mem));
    memset(wmem,0,sizeof(struct _wcomp_mem));
    
    wmem->tkz2_mem=init_tkz2_mem();
    wmem->lnav=init_lnav();
    
    wmem->size_cat_entry=1000;
    wmem->cat_entry=(char *)L_ALLOC(sizeof(char)*wmem->size_cat_entry);
    
    return wmem;
}

void free_wcomp_mem(WCOMP_MEM wmem){
    if (wmem->rcount>1){
        wmem->rcount--;
        return;
    }
    
    free_tkz2_mem(wmem->tkz2_mem);
    free_lnav(wmem->lnav);
    
    L_FREE(wmem->cat_entry);
    
    L_FREE(wmem);
}

void display_match_wcomp_mem(WCOMP_MEM wmem){
    
    char entry_info_buff[100];
    char *entry_info=NULL;
    if (wmem->entry_info){
        entry_info=wmem->entry_info;
    }
    else{
        entry_info_buff[0]='\0';
        entry_info=entry_info_buff;
    }
    
    fprintf(stdout,
            "[%ld(%ld)-%ld(%ld)] %s : %s\n",
            (unsigned long)wmem->i,
            wmem->char_start,
            (unsigned long)wmem->j,
            wmem->char_end,
            wmem->cat_entry,
            entry_info);
}


#pragma mark - EID or COL_ID to ECID and back


uint32_t col_id_to_ecid(uint32_t col_id){
    ei_or_col_id ec;
    ec.i=col_id;
    ec.bytes[3]=ec.bytes[3] |  0x80;
    return ec.i;
}

uint32_t get_eid_or_col_id_from_uint(uint32_t ecid,
                                     int *p_is_entry_id){
    ei_or_col_id ec;
    ec.i=ecid;
    
    if (ec.bytes[3]<0){
        ec.bytes[3]=(ec.bytes[3] & ~0x80);
        *p_is_entry_id=0;
        return ec.i;
    }
    else{
        *p_is_entry_id=1;
        return ecid;
    }

    return 0;
}

#pragma mark - RUNTIME TOP

// Handles both comp->string and comp->L5 code
int first_match_main_wcomp(WCOMP wcomp,
                           char *ln,
                           LSPACE lspace,
                           WCOMP_MEM wmem,
                           wcomp_match_type match_type){
    
    if (lspace && lspace->nb_words<=2)
        return 0; // needs at least 3 (two sentence boundaries plus one word)
    
    if (wmem->size_cat_entry<=wcomp->max_cat_entry_length){
        L_FREE(wmem->cat_entry);
        wmem->cat_entry=(char *)L_ALLOC(sizeof(char)*(wcomp->max_cat_entry_length+100));
    }
    
    wmem->runtime_state=wrs_out;
    wmem->last_real_match_j=0;
    return next_match_main_wcomp(wcomp, ln, lspace,wmem, match_type);

}

int first_match_wcomp(WCOMP wcomp,
                     char *ln,
                     WCOMP_MEM wmem,
                     wcomp_match_type match_type){
    
    return first_match_main_wcomp(wcomp,
                                  ln,
                                  NULL,
                                  wmem,
                                  match_type);
    
}


int next_match_wcomp(WCOMP wcomp,
                     char *ln,
                     WCOMP_MEM wmem,
                     wcomp_match_type match_type){
    
    return next_match_main_wcomp(wcomp,
                                 ln,
                                 NULL,
                                 wmem,
                                 match_type);
}


int next_match_main_wcomp(WCOMP wcomp,
                          char *ln,
                          LSPACE lspace,
                          WCOMP_MEM wmem,
                          wcomp_match_type match_type){
    
    // sets longest match flag
    int longest_match=0;
    switch (match_type) {
        case wmt_longest_matches_strict:
        case wmt_longest_matches:
            longest_match=1;
            break;
            
        default:
            break;
    }
    
    
    if (wmem->runtime_state==wrs_out){
        
        if (!lspace){
            
            tokenize_string_tkz3(wcomp->tkz3,
                                 wmem->tkz2_mem,
                                 wmem->lnav,
                                 (unsigned char *)ln,
                                 0,
                                 0,
                                 0);
            
            if (!wmem->lnav->nb_tokens)
                return 0;
            
            wmem->i=0;wmem->j=0;

        }
        else{
            // Takes into account the sentence boundary
            wmem->i=1;wmem->j=1;
        }
        
        
        wmem->runtime_state=wrs_match_at_pos;
        
    }
    // For locality
    
    int has_simple_words=wcomp->has_simple_words;
    uint32_t max_word_length=wcomp->max_word_length;
    
    uint32_t i=wmem->i;
    uint32_t j=wmem->j;
    uint32_t last_real_match_j=wmem->last_real_match_j;
    
    LTOKEN *tokens=NULL;
    unsigned long nb_tokens=0;
    LBLOCK *blocks=NULL;
    LPL_ISYMB isymb=NULL;
    unsigned long max_j=0;
    
    if (!lspace){
        tokens=(LTOKEN *)wmem->lnav->tokens->array;
        nb_tokens=wmem->lnav->nb_tokens;
        max_j=nb_tokens-1;
    }
    else{
        blocks=(LBLOCK *)lspace->word_block_at_pos->array;
        nb_tokens=lspace->nb_words-1; // doesn't cound the right/end of sentence marker
        isymb=lspace->isymb;
        max_j=nb_tokens;
    }
    
    wcomp_runtime_state state=wmem->runtime_state;
    
    uint32_t size_tab_w1=wcomp->size_tab_w1;
    uint32_t *tab_w1=wcomp->tab_w1;
    
    uint32_t bit_size_tab_w2=wcomp->bit_size_tab_w2;
    
    char *tab_w2=wcomp->tab_w2;

    uint32_t bit_size_tab_p1=wcomp->bit_size_tab_p1;
    uint32_t bit_size_tab_p2=wcomp->bit_size_tab_p2;
    char *tab_p1=wcomp->tab_p1;
    char *tab_p2=wcomp->tab_p2;
    
    uint32_t *collision_sets=wcomp->collision_sets;
    uint32_t *col_set_pos=wcomp->col_set_pos;

    char *entries=wcomp->entries;
    uint32_t *entry_id_to_entries_pos=wcomp->entry_id_to_entries_pos;
    
    uint32_t h=0,hb=0;  // two independant hash values; shared between words and prefixes
    uint32_t iw;   // indexes for words
    uint32_t ip; // index for prefix
    
    unsigned char *word=NULL;
    
    // Non-0 number representing either the entry_id or collision_set id
    // depending on the higher order bit
    
    uint32_t ecid=0; // entry_id or collision_id
    uint32_t entry_id=0;
    uint32_t ec2=0;
    uint32_t prefix_len=0;

    int is_entry_id;
    int is_entry_correct=0;

    uint32_t previous_matched_j=UINT32_MAX;
    char *previous_ln_end_of_cat_entry=NULL;
    
    if (state==wrs_extend_match){
        
        if (j+1>=nb_tokens){
            
            if (i+1>=nb_tokens)
                return 0;
            
            i++;j=i;
            state=wrs_match_at_pos;
//            
//            return 0;
        }
        else{
        
            j=j+1;
            state=wrs_test_match_then_prefix;
            
            // Gets the word
            if (tokens){
                LTOKEN ltoken=tokens[j];word=get_fast_word_ltoken(ltoken);
            }
            else{
                LBLOCK block=blocks[j];word=get_string_from_id_lpl_isymb(isymb,block->string_id);
            }
            
            h=wmem->h;
            hb=wmem->hb;
            
            h=fnv1a_extend_str((unsigned char *)"/", h);
            h=fnv1a_extend_str(word, h);
            hb=fnv1a_extend_str((unsigned char *)"/", hb);
            hb=fnv1a_extend_str(word, hb);
        }

    }
    else if (state==wrs_next_match){
        
        if (match_type==wmt_longest_matches_strict){
            i=j+1;j=i;
            state=wrs_match_at_pos;
        }
        else if (match_type==wmt_longest_matches){
            i=i+1;j=i;
            state=wrs_match_at_pos;
        }

    }
    
    while(1){
        
        if (j>=nb_tokens){
            wmem->runtime_state=wrs_finished;
            
            if (longest_match && previous_matched_j!=UINT32_MAX){
                j=previous_matched_j;
                *previous_ln_end_of_cat_entry='\0'; // makes sure it's just the last one
                state=wrs_match_to_return;
            }
            else{
                return 0;
            }
        }
        
        switch (state) {
            case wrs_match_at_pos:
            {
                // Gets the word
                if (tokens){
                    LTOKEN ltoken=tokens[i];word=get_fast_word_ltoken(ltoken);
                }
                else{
                    LBLOCK block=blocks[i];
                    if (!block){
                        // defensive coding
                        state=wrs_finished;
                        fprintf(stderr, "WARNING: next_match_main_wcomp/1 Should not sbe there.\n");
                        break;
                    }
                    word=get_string_from_id_lpl_isymb(isymb,block->string_id);
                }

                // Initializes the hash
                h=fnv1a_str(word);
                hb=fnv1a_B_str(word);
                previous_matched_j=UINT32_MAX;
                prefix_len=1;
                
                if (has_simple_words)
                    state=wrs_test_match_then_prefix;
                else
                    state=wrs_match_failed_try_prefix;

            }
                break;
            case wrs_test_match_then_prefix:
            {
                
                if (longest_match && last_real_match_j && j<=last_real_match_j){
                    // Doesn't go beyond the previous match
                    state=wrs_match_failed_try_prefix;
                    continue;
                }

                    
                iw=hb % bit_size_tab_w2;
                
                if (!BIT_AT_CHAR_TABLE(tab_w2, iw)){
                    
                    // Not a word match. Tests for prefix
                    
                    state=wrs_match_failed_try_prefix;
                    continue;
                    
                }
                else{
                    // Could be a word; does the second hash test
                    
                    iw=h % size_tab_w1;
                    
                    if (!(ecid=tab_w1[iw])){
                        
                        // Not a word match. Tests for prefix
                        
                        state=wrs_match_failed_try_prefix;
                        continue;
                        
                    }
                    else{
                        // Checks that the word is the same
                        
                        // - Has to build the concatenated string '/' separated
                        
                        char *ln_cat_entry=wmem->cat_entry;
                        for(unsigned long k=i;k<=j;k++){
                            if (k>i)
                                *ln_cat_entry++='/';
                            
                            char *word2=NULL;
                            if (tokens){
                                word2=(char *)get_fast_word_ltoken(tokens[k]);
                            }
                            else{
                                LBLOCK block=blocks[k];word2=(char *)get_string_from_id_lpl_isymb(isymb,block->string_id);
                            }
                            
                            for(char *ln=word2;*ln!='\0';) {*ln_cat_entry++ = *ln++;};
                        }
                        *ln_cat_entry='\0';
                        
                        is_entry_correct=1;
                        
                        ec2=get_eid_or_col_id_from_uint(ecid,&is_entry_id);
                        
                        if (is_entry_id){
                            entry_id=ec2;
                            if (strcmp((const char *)wmem->cat_entry,
                                       &(entries[entry_id_to_entries_pos[entry_id]]))){
                                
                                is_entry_correct=0;
                            }

                        }
                        else{
                            // Has to iterate through the collision to find whether the entry
                            // exists
                            is_entry_correct=0;

                            for(uint32_t *pentry_id=&(collision_sets[col_set_pos[ec2]]);
                                *pentry_id!=0;
                                pentry_id++){
                                
                                entry_id=*pentry_id;
                                if (!strcmp((const char *)wmem->cat_entry,
                                           &(entries[entry_id_to_entries_pos[entry_id]]))){
                                    is_entry_correct=1;
                                    break;
                                }
                            }
                        }
                        
                        if (!is_entry_correct){
                            // Not a word match. Tests for prefix
                            
                            state=wrs_match_failed_try_prefix;
                            continue;
                        }
                        else{
                            // This is a match
                            
                            if (longest_match){
                                previous_matched_j=j;
                                previous_ln_end_of_cat_entry=ln_cat_entry;
                                
                                j++;
                                
                                if (j>=nb_tokens){
                                    j--;
                                    state=wrs_match_to_return;
                                    continue;
                                }
                                else{
                                    
                                    // Gets the word
                                    if (tokens){
                                        LTOKEN ltoken=tokens[j];word=get_fast_word_ltoken(ltoken);
                                    }
                                    else{
                                        LBLOCK block=blocks[j];word=get_string_from_id_lpl_isymb(isymb,block->string_id);
                                    }

                                    h=fnv1a_extend_str((unsigned char *)"/", h);
                                    h=fnv1a_extend_str(word, h);
                                    hb=fnv1a_extend_str((unsigned char *)"/", hb);
                                    hb=fnv1a_extend_str(word, hb);
                                    
                                    prefix_len++;
                                    
                                    state=wrs_test_match_then_prefix;
                                }
                            }
                            else{
                                state=wrs_match_to_return;
                                continue;
                            }
                        }
                    }
                }
            }
                break;
            case wrs_match_failed_try_prefix:
            {
                if (!bit_size_tab_p1 || j+1>=nb_tokens || prefix_len>=max_word_length){
                    // NOT A PREFIX
                    
                    if (longest_match && previous_matched_j!=UINT32_MAX){
                        j=previous_matched_j;
                        *previous_ln_end_of_cat_entry='\0'; // makes sure it's just the last one
                        state=wrs_match_to_return;
                        continue;
                    }
                    else{
                        i++;j=i;
                        state=wrs_match_at_pos;
                        continue;
                    }
                }
                
                
                ip = h % bit_size_tab_p1;
                
                if (!BIT_AT_CHAR_TABLE(tab_p1, ip)){
                    
                    // NOT A PREFIX
                    
                    if (longest_match && previous_matched_j!=UINT32_MAX){
                        j=previous_matched_j;
                        state=wrs_match_to_return;
                        continue;
                    }
                    else{
                        i++;j=i;
                        state=wrs_match_at_pos;
                        continue;
                    }
                }
                else{
                    // Might be a prefix; does the prefix second text
                    ip = hb % bit_size_tab_p2;
                    
                    if (!BIT_AT_CHAR_TABLE(tab_p2, ip)){
                        // NOT A PREFIX
                        
                        if (longest_match && previous_matched_j!=UINT32_MAX){
                            j=previous_matched_j;
                            state=wrs_match_to_return;
                            continue;
                        }
                        else{
                            i++;j=i;
                            state=wrs_match_at_pos;
                            continue;
                        }
                    }
                    else{
                        // Assumes it's a prefix (with high probability)
//                        j=i+1;  WEIRD
                        j++;
                        
                        // Gets the word
                        if (tokens){
                            LTOKEN ltoken=tokens[j];word=get_fast_word_ltoken(ltoken);
                        }
                        else{
                            LBLOCK block=blocks[j];word=get_string_from_id_lpl_isymb(isymb,block->string_id);
                        }
                        
                        h=fnv1a_extend_str((unsigned char *)"/", h);
                        h=fnv1a_extend_str(word, h);
                        hb=fnv1a_extend_str((unsigned char *)"/", hb);
                        hb=fnv1a_extend_str(word, hb);
                        
                        prefix_len++;
                        
                        state=wrs_test_match_then_prefix;
                        continue;
                    }
                }
            }
                break;
            case wrs_match_to_return:
            {
                if (longest_match){
                    wmem->runtime_state=wrs_next_match;
                    wmem->last_real_match_j=j;
                }
                else
                    wmem->runtime_state=wrs_extend_match;
                
                wmem->i=i;
                wmem->j=j;
                
                if (tokens){
                    LTOKEN token_start=tokens[i];
                    LTOKEN token_end=tokens[j];
                    wmem->char_start=token_start->char_start;
                    wmem->char_end=token_end->char_end;
                }
                else{
                    LBLOCK block_start=blocks[i];
                    LBLOCK block_end=blocks[j];
                    wmem->char_start=block_start->char_start;
                    wmem->char_end=block_end->char_end;
                }
                
                wmem->h=h;
                wmem->hb=hb;
                
                if (wcomp->entry_info_num)
                    wmem->entry_info_num=wcomp->entry_info_num[entry_id];
                else
                    wmem->entry_info_num=0;
                
                if (wcomp->entries_info)
                    wmem->entry_info=&(wcomp->entries_info[wcomp->entry_id_to_entry_info_pos[entry_id]]);
                else
                    wmem->entry_info=NULL;
                
                return 1;

            }
                break;
                
            default:
                break;
        }
        
    }
    
    
    return 1;
}

