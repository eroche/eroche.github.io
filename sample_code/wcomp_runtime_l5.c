// wcomp_runtime_l5.c

#include"lang.h"

#pragma mark - Local Prototypes


int apply_create_single_typed_block_range_l5(LSPACE lspace,
                                             unsigned long start,
                                             unsigned long end,
                                             unsigned long string_id,
                                             lblock_type block_type,
                                             int is_unambiguous);

int apply_disable_subparts_range_l5(LSPACE lspace,
                                    unsigned long start,
                                    unsigned long end,
                                    LGPACK_MEM mem);

int apply_glue_range_l5(LSPACE lspace,
                        unsigned long start,
                        unsigned long end,
                        int is_unambiguous,
                        lblock_type block_type,
                        int do_not_insert_space,
                        LPL_ISYMB isymb,
                        LGPACK_MEM mem);

int apply_set_unambiguous_last_range_l5(LSPACE lspace);

int apply_set_min_weight_last_range_l5(LSPACE lspace,
                                       unsigned long weight);

int add_tag_last_range_l5(LSPACE lspace,unsigned long tag_id);

int add_at_val_last_range_l5(LSPACE lspace,
                             unsigned long at_id,
                             lblock_atval_type tp,
                             uint32_t val_int,
                             float val_float,
                             unsigned long val_string_id);

int set_match_as_last_range_l5(LSPACE lspace,
                               unsigned long start,
                               unsigned long end,
                               unsigned long index_of_block_to_set_as_last,
                               int set_as_unambiguous);

int set_block_as_last_range_l5(LSPACE lspace,
                               LBLOCK lblock,
                               int set_at_unambiguous);


#pragma mark - L5 RUNTIME TOP LEVEL

void runtime_wcomp_l5(LSPACE lspace,
                      WCOMP wcomp,
                      LPL_ISYMB isymb,
                      LGPACK_MEM mem){
    
    L5_EXE_O l5_exe_o=wcomp->clover_exe->l5_exe_o;
    
    if (!l5_exe_o)
        return;
    
    if (!wcomp->on_whole_lspace){
        
        // Applies only on tokenized words
        
        WCOMP_MEM wmem=get_wcomp_mem_lgpack_mem(mem);
        
        for(int t=first_match_main_wcomp(wcomp, NULL, lspace, wmem, wmt_all_matches);
            t;
            t=next_match_main_wcomp(wcomp, NULL, lspace, wmem, wmt_all_matches)){
            
            unsigned long segment_id=(unsigned long)wmem->entry_info_num;
            
            if (!segment_id)
                continue;
            
            apply_code_segment_l5_for_range(lspace,
                                            wmem->i,
                                            wmem->j,
                                            NULL,
                                            l5_exe_o,
                                            isymb,
                                            segment_id,
                                            mem);
            
        }
    }
    else{
        
        // Applies on all individual blocks (possibly of a given type)
        
        runtime_all_single_blocks_wcomp(lspace,
                                        wcomp,
                                        isymb,
                                        mem);
    }
    
}

#pragma mark - APPLYING CODE SEGMENT

// Applies L5 code when there is no matching block but a matching range
void apply_code_segment_l5_for_range(LSPACE lspace,
                                     unsigned long start_match,
                                     unsigned long end_match,
                                     LBLOCK matched_block, // this is optional: if a known single block is the match
                                     L5_EXE_O l5_exe_o,
                                     LPL_ISYMB isymb,
                                     unsigned long code_id,
                                     LGPACK_MEM lgpack_mem){
    
    unsigned long *code=NULL;
    
    l5_op op;
    unsigned long num;
    unsigned long code_buff[10];
    unsigned long code_len=0;
    unsigned long arg2,arg3;
    
    if (get_op_and_arg_from_atomic_code((uint32_t)code_id, &op, &num,&arg2,&arg3)){
        code_buff[0]=op;
        code_buff[1]=num;
        code_buff[2]=arg2;
        code_buff[3]=arg3;
        code_len=4;
        code=code_buff;
    }
    else{
        code_len=get_code_segment_l5_exe_o(l5_exe_o, code_id, &code);
    }
    
    if (!code_len){
        // no code for this segment: does nothing
        return;
    }
    
    unsigned long pos=0;
    
    l5_op the_op;
    unsigned long arg1;
    
    // Variables (can be added for operators with more than 3 arguments)
    unsigned long v1=0;
    int has_v1=0;
    
    
    while(pos<code_len){
        
        grab_quad_l5(&(code[pos]), &the_op, &arg1, &arg2, &arg3);pos+=4;
        
        switch (the_op) {
                
            case op_create_single_typed_block:
                {
                    apply_create_single_typed_block_range_l5(lspace,
                                                             start_match,
                                                             end_match,
                                                             arg1, // label_id
                                                             (lblock_type)arg2,
                                                             (int)arg3); // is_unambiguous
                }
                break;
            case op_set_var_wcomp_runtime:
            {
                if (arg1==1){
                    v1=arg2;
                    has_v1=1;
                }
            }
                break;
            case op_create_single_typed_block_at_range:
            {
                unsigned long sub_range_start=(unsigned long)arg2;
                unsigned long sub_range_end=(unsigned long)arg3;
                
                lblock_type bt=(lblock_type)((has_v1) ? v1 : 0);
                
                apply_create_single_typed_block_range_l5(lspace,
                                                         start_match+sub_range_start,
                                                         start_match + sub_range_end,
                                                         arg1, // label_id
                                                         bt,
                                                         0); // is_unambiguous
                
                has_v1=0;


            }
                break;
            case op_create_unambiguous_block:
                {
                    apply_create_single_typed_block_range_l5(lspace,
                                                             start_match,
                                                             end_match,
                                                             arg1,
                                                             (lblock_type)arg2,
                                                             1);

                }
                break;
            case op_glue:
                {
                    apply_glue_range_l5(lspace,
                                        start_match,
                                        end_match,
                                        (int)arg1,
                                        (int)arg2,
                                        (int)arg3,
                                        isymb,
                                        lgpack_mem);
                }
                break;
            case op_glue_range:
            {
                unsigned long sub_range_start=(unsigned long)arg1;
                unsigned long sub_range_end=(unsigned long)arg2;
                
                apply_glue_range_l5(lspace,
                                    start_match + sub_range_start,
                                    start_match + sub_range_end,
                                    0, // not unambiuguous
                                    (int)arg3,
                                    0, // insert space
                                    isymb,
                                    lgpack_mem);

            }
                break;
            case op_glue_no_space_range:
            {
                unsigned long sub_range_start=(unsigned long)arg1;
                unsigned long sub_range_end=(unsigned long)arg2;
                
                apply_glue_range_l5(lspace,
                                    start_match + sub_range_start,
                                    start_match + sub_range_end,
                                    0, // not unambiguous
                                    (int)arg3,
                                    1, // do not insert space
                                    isymb,
                                    lgpack_mem);

            }
                break;
            case op_force_two_pos:
                {
                    
                }
                break;
            case op_disable_subparts:
                {
                    apply_disable_subparts_range_l5(lspace, start_match, end_match, lgpack_mem);
                }
                break;
            case op_set_unambiguous_last:
            {
                apply_set_unambiguous_last_range_l5(lspace);
            }
                break;

            case op_set_min_weight_last:
            {
                apply_set_min_weight_last_range_l5(lspace,arg1);
            }
                break;
            case op_add_tag_last:
            {
                add_tag_last_range_l5(lspace, arg1);
            }
                break;
            case op_add_at_val_num_last:
            {
                add_at_val_last_range_l5(lspace, arg1, baty_int, (uint32_t)arg2, 0, 0);
            }
                break;
            case op_add_at_val_float_last:
            {
                float f=0.0;uint32_t ff=(uint32_t)arg2;memcpy(&f,&ff, sizeof(float));

                add_at_val_last_range_l5(lspace, arg1, baty_float, 0, f, 0);
            }
                break;
            case op_add_at_val_string_last:
            {
                add_at_val_last_range_l5(lspace, arg1, baty_string, 0, 0.0, arg2);
            }
                break;
            case op_set_match_as_last:
            {
                if (!matched_block)
                    set_match_as_last_range_l5(lspace,start_match,end_match,arg1,(int)arg2);
                else{
                    // the block on which to apply things is known
                    set_block_as_last_range_l5(lspace, matched_block, (int)arg2);
                }
            }
                break;
            default:
                break;
        }
    }
}

#pragma mark - L5 atomic operation on RANGES

int apply_create_single_typed_block_range_l5(LSPACE lspace,
                                             unsigned long start,
                                             unsigned long end,
                                             unsigned long string_id,
                                             lblock_type block_type,
                                             int is_unambiguous){
    
    LBLOCK start_block=lspace->word_block_at_pos->array[start];
    LBLOCK end_block=lspace->word_block_at_pos->array[end];
    
    if (!start_block || !end_block)
        return 0;
    
    LBLOCK block1 =init_with_falmem_lblock(lspace->falmem,lspace->isymb);
    block1->tp=block_type;
    block1->sentence_word_offset=start_block->sentence_word_offset;
    block1->word_start=start;
    block1->word_end=end;
    block1->char_start=start_block->char_start;
    block1->char_end=end_block->char_end;
    
    block1->string_id=string_id;
    
    if (is_unambiguous)
        block1->ambiguity=lblock_ambiguity_unambiguous;
    
    add_block_lspace(lspace, block1,1);
    
    return 1;
}

int apply_disable_subparts_range_l5(LSPACE lspace,
                                    unsigned long start,
                                    unsigned long end,
                                    LGPACK_MEM mem){
    
    unsigned long nb_disables=get_blocks_strictly_within_range_lspace(lspace,
                                                                      mem->dp1,
                                                                      start,
                                                                      end,
                                                                      mem);
    
    for(unsigned long i=0;i<nb_disables;i++){
        
        LBLOCK block_to_disable=(LBLOCK)get_at_dyn_pointer_array(mem->dp1, i);
        disable_block_lspace(lspace, block_to_disable);
        
    }

    return 1;
}

// Creates a new block from start to end with label the concatenation
// of the words from start to end
int apply_glue_range_l5(LSPACE lspace,
                        unsigned long start,
                        unsigned long end,
                        int is_unambiguous,
                        lblock_type block_type,
                        int do_not_insert_space,
                        LPL_ISYMB isymb,
                        LGPACK_MEM mem){
    
    LBLOCK start_block=lspace->word_block_at_pos->array[start];
    LBLOCK end_block=lspace->word_block_at_pos->array[end];
    
    if (!start_block || !end_block)
        return 0;

    
    char buff[1000];*buff='\0';
    DYN_CHAR_ARRAY dca=NULL;uint64_t index=0;

    if (start>end)
        return 0;
    
    for(unsigned long i=start;i<=end;i++){
        
        LBLOCK block=lspace->word_block_at_pos->array[i];
        
        if (!block)
            break;
        
        unsigned long string_id=block->string_id;
        char *word=(char *)get_string_from_id_lpl_isymb(isymb,string_id);
        
        if (!word)
            break;
        
        if (!do_not_insert_space && i>start){
            strcpy_buff_or_dyn_char_array(buff, 1000, &dca, &index, " ");
        }

        strcpy_buff_or_dyn_char_array(buff, 1000, &dca, &index,word);
        //        strcat(buff,word);
    }
    
    char *new_word=(dca) ? (char *)dca->array : buff;
    
    if (*new_word=='\0')
        return 0;
    
    unsigned long compound_string_id=add_pure_string_if_absent_lpl_isymb(lspace->isymb,
                                                                         (unsigned char *)new_word);
    
    LBLOCK block1 =init_with_falmem_lblock(lspace->falmem,lspace->isymb);
    
    
    block1->tp=(block_type==lblock_tp_unknown) ? lblock_tp_word : block_type;
    
    block1->sentence_word_offset=start_block->sentence_word_offset;
    block1->word_start=start;
    block1->word_end=end;
    block1->char_start=start_block->char_start;
    block1->char_end=end_block->char_end;
    
    block1->string_id=compound_string_id;
    
    if (is_unambiguous)
        block1->ambiguity=lblock_ambiguity_unambiguous;
    
    add_block_lspace(lspace, block1,1);
    
    return 1;
}

int apply_set_unambiguous_last_range_l5(LSPACE lspace){
    if (!lspace->nb_blocks)
        return 0;
    
    LBLOCK block=NULL;
    if (lspace->last_added_block_id && lspace->last_added_block_id<lspace->nb_blocks)
        block=lspace->blocks->array[lspace->last_added_block_id];
    else
        block=lspace->blocks->array[lspace->nb_blocks-1];
    
    block->ambiguity=lblock_ambiguity_unambiguous;
    
    return 1;
}

// Applies the min weight to the last block that was created
int apply_set_min_weight_last_range_l5(LSPACE lspace,
                                       unsigned long weight){
    
    if (!lspace->nb_blocks)
        return 0;
    
    LBLOCK block=NULL;
    if (lspace->last_added_block_id && lspace->last_added_block_id<lspace->nb_blocks)
        block=lspace->blocks->array[lspace->last_added_block_id];
    else
        block=lspace->blocks->array[lspace->nb_blocks-1];

    if (block->weight<weight)
        block->weight=weight;
    
    return 1;
    
}

int add_tag_last_range_l5(LSPACE lspace,unsigned long tag_id){
    
    if (!lspace->nb_blocks)
        return 0;
    
    LBLOCK block=NULL;
    if (lspace->last_added_block_id && lspace->last_added_block_id<lspace->nb_blocks)
        block=lspace->blocks->array[lspace->last_added_block_id];
    else
        block=lspace->blocks->array[lspace->nb_blocks-1];

    add_tag_lblock(block, tag_id);

    return 1;
}

int add_at_val_last_range_l5(LSPACE lspace,
                             unsigned long at_id,
                             lblock_atval_type tp,
                             uint32_t val_int,
                             float val_float,
                             unsigned long val_string_id){
    
    if (!lspace->nb_blocks)
        return 0;
    
    LBLOCK block=NULL;
    if (lspace->last_added_block_id && lspace->last_added_block_id<lspace->nb_blocks)
        block=lspace->blocks->array[lspace->last_added_block_id];
    else
        block=lspace->blocks->array[lspace->nb_blocks-1];

    add_at_val_lblock(block, at_id, tp, val_int, val_float, val_string_id);

    return 1;
}

// if i=index_of_block_to_set_as_last
// then sets the first block at start+i as the last added block
int set_match_as_last_range_l5(LSPACE lspace,
                               unsigned long start,
                               unsigned long end,
                               unsigned long index_of_block_to_set_as_last,
                               int set_as_unambiguous){
    
    unsigned long pos;
    LBLOCK lblock=NULL;
    
    if (!first_cell_p_llist(lspace->blocks_starting_at_word_pos,
                            start+index_of_block_to_set_as_last,
                            (void **)&lblock,
                            &pos))
        return 0;
    
    lspace->last_added_block_id=lblock->block_id;
    
    if (set_as_unambiguous)
        lblock->ambiguity=lblock_ambiguity_unambiguous;
    
    return 1;
}

int set_block_as_last_range_l5(LSPACE lspace,
                               LBLOCK lblock,
                               int set_as_unambiguous){
    
    lspace->last_added_block_id=lblock->block_id;

    if (set_as_unambiguous)
        lblock->ambiguity=lblock_ambiguity_unambiguous;

    return 1;

}
