// fst_d_444.c

#include"lang.h"

#pragma mark - Init

void free_fst_d_444(FST_D_444 fst){
    
    if (fst->rcount>1){
        fst->rcount--;
        return;
    }
    
    if (fst->i_symb)
        free_symb_o(fst->i_symb);
    
    if (fst->o_symb)
        free_symb_o(fst->o_symb);
    
    L_FREE(fst->state_types);
    L_FREE(fst->state_pos);
    
    if (fst->size_direct_access_table){
        L_FREE(fst->ilabel_table);
        L_FREE(fst->olabel_table);
        L_FREE(fst->arr_state_table);
        L_FREE(fst->extra_trans_pos_table);
    }
    
    if (fst->size_extra_transitions_table){
        L_FREE(fst->extra_transitions_table);
    }
    
    L_FREE(fst);
}

void write_fst_d_444(FILE *out,FST_D_444 fst){
    
    write_long(out, T_FST_D_444);
    write_long(out, V_FST_D_444);
    write_long(out, size_of_fst_d_444(fst));
    
    write_long(out,fst->max_ilabel);
    
    write_long(out,fst->nb_states);
    
    if (fst->nb_states){
        fwrite(fst->state_types, sizeof(long), fst->nb_states, out);
        fwrite(fst->state_pos, sizeof(long), fst->nb_states, out);
    }
    
    write_long(out,fst->size_direct_access_table);
    
    if (fst->size_direct_access_table){
        fwrite(fst->ilabel_table, sizeof(long), fst->size_direct_access_table, out);
        fwrite(fst->olabel_table, sizeof(long), fst->size_direct_access_table, out);
        fwrite(fst->arr_state_table, sizeof(long), fst->size_direct_access_table, out);
        fwrite(fst->extra_trans_pos_table, sizeof(long), fst->size_direct_access_table, out);
    }
    
    write_long(out,BINARY_TAG);
    
    write_long(out,fst->size_extra_transitions_table);
    
    if (fst->size_extra_transitions_table){
        fwrite(fst->extra_transitions_table, sizeof(long), fst->size_extra_transitions_table, out);
    }

    
    if (fst->i_symb && fst->i_symb->write){
        write_long(out, 1);
        write_SYMB_O(fst->i_symb, out);
    }
    else
        write_long(out, 0);
    
    
    if (fst->o_symb && fst->o_symb->write){
        write_long(out, 1);
        write_SYMB_O(fst->o_symb, out);
    }
    else
        write_long(out, 0);

    
    write_long(out,BINARY_TAG);


}

FST_D_444 read_fst_d_444(FILE *in){
    if (!in)
        return NULL;
    
    unsigned long tp=read_long(in);
    /*unsigned long version=*/read_long(in);
    /*unsigned long size=*/read_long(in);
    
    if (tp!=T_FST_D_444){
        fprintf(stderr, "ERROR: Incorrect file type %s,%d\n",__FILE__,__LINE__);
        return NULL;
    }

    FST_D_444 fst=(FST_D_444)L_ALLOC(sizeof(struct _fst_d_444));
    memset(fst,0,sizeof(struct _fst_d_444));
    
    fst->max_ilabel=read_long(in);
    fst->nb_states=read_long(in);
    
    if (fst->nb_states){
        fst->state_types=(unsigned long *)L_ALLOC(sizeof(long)*fst->nb_states);
        fread(fst->state_types, sizeof(long), fst->nb_states, in);
        
        fst->state_pos=(unsigned long *)L_ALLOC(sizeof(long)*fst->nb_states);
        fread(fst->state_pos, sizeof(long), fst->nb_states, in);
    }
    
    fst->size_direct_access_table=read_long(in);
    
    if (fst->size_direct_access_table){
        fst->ilabel_table=(unsigned long *)L_ALLOC(sizeof(long)*fst->size_direct_access_table);
        fread(fst->ilabel_table, sizeof(long), fst->size_direct_access_table, in);
        
        fst->olabel_table=(unsigned long *)L_ALLOC(sizeof(long)*fst->size_direct_access_table);
        fread(fst->olabel_table, sizeof(long), fst->size_direct_access_table, in);
        
        fst->arr_state_table=(unsigned long *)L_ALLOC(sizeof(long)*fst->size_direct_access_table);
        fread(fst->arr_state_table, sizeof(long), fst->size_direct_access_table, in);
        
        fst->extra_trans_pos_table=(unsigned long *)L_ALLOC(sizeof(long)*fst->size_direct_access_table);
        fread(fst->extra_trans_pos_table, sizeof(long), fst->size_direct_access_table, in);
    }
    
    check_binary_tag(in, "read_fst_d_444 (1)");

    
    fst->size_extra_transitions_table=read_long(in);
    
    if (fst->size_extra_transitions_table){
        fst->extra_transitions_table=(unsigned long *)L_ALLOC(sizeof(long)*fst->size_extra_transitions_table);
        fread(fst->extra_transitions_table, sizeof(long), fst->size_extra_transitions_table, in);
    }
    
    if (read_long(in)){
        fst->i_symb=read_symb_o(in);
        fst->i_symb->rcount=1;
    }
    
    if (read_long(in)){
        fst->o_symb=read_symb_o(in);
        fst->o_symb->rcount++;
    }
    
    check_binary_tag(in, "read_fst_d_444 (2)");

    return fst;
}


unsigned long size_of_fst_d_444(FST_D_444 fst){
    unsigned long the_size=0;
    
    the_size += sizeof(struct _fst_d_444);
    
    the_size+=sizeof(long)*fst->nb_states; // state_types
    the_size+=sizeof(long)*fst->nb_states; // state_pos
    
    if (fst->size_direct_access_table){
        the_size+=sizeof(long)*fst->size_direct_access_table; // ilabel_table
        the_size+=sizeof(long)*fst->size_direct_access_table; // olabel_table
        the_size+=sizeof(long)*fst->size_direct_access_table; // arr_state_table
        
        if (fst->extra_trans_pos_table)
            the_size+=sizeof(long)*fst->size_direct_access_table; // extra_trans_pos_table
    }

    if (fst->size_extra_transitions_table){
        the_size+=sizeof(long)*fst->size_extra_transitions_table; // extra_transitions_table
    }
    
    return the_size;
}

void display_mem_fst_d_444(int depth,FST_D_444 fst){
    
    char prefix[100];
    
    int i=0;
    for(;i<depth && i<99;i++)
        prefix[i]='.';
    prefix[i]='\0';
    
    fprintf(stdout, "%s FST_D_444: %ld\n",prefix,size_of_fst_d_444(fst));
    fprintf(stdout, "%s nb_states: %ld\n",prefix,fst->nb_states);
    fprintf(stdout, "%s size_direct_access_table: %ld\n",prefix,fst->size_direct_access_table);

    if (fst->size_direct_access_table){
        fprintf(stdout, "%s ilabel_table: %ld\n",prefix,sizeof(long)*fst->size_direct_access_table);
        fprintf(stdout, "%s olabel_table: %ld\n",prefix,sizeof(long)*fst->size_direct_access_table);
        fprintf(stdout, "%s arr_state_table: %ld\n",prefix,sizeof(long)*fst->size_direct_access_table);
        
        if (fst->extra_trans_pos_table)
            fprintf(stdout, "%s extra_trans_pos_table: %ld\n",prefix,sizeof(long)*fst->size_direct_access_table);
    }
    
    if (fst->size_extra_transitions_table){
        fprintf(stdout, "%s extra_transitions_table: %ld\n",prefix,sizeof(long)*fst->size_extra_transitions_table);
    }
    
}

#pragma mark - Building

// Used to put states that have the most transitions first
// because they are the most complex to place
int compare_state_trans_for_qsort(const void *t1,const void *t2){
    unsigned long *trans1=(unsigned long *)t1;
    unsigned long state_id1=trans1[0];
    unsigned long nb_trans1=trans1[1];
    
    unsigned long *trans2=(unsigned long *)t2;
    unsigned long state_id2=trans2[0];
    unsigned long nb_trans2=trans2[1];
    
    if (state_id1==0)
        return -1;
    
    if (state_id2==0)
        return 1;
    
    if (nb_trans1>nb_trans2)
        return -1;
    else if (nb_trans1<nb_trans2)
        return 1;
    else if (state_id1<state_id2)
        return -1;
    else if (state_id1>state_id2)
        return 1;
    else
        return 0;
    
    return 0;
}

// Puts the ilabels in order such that dubplicates can be removed
int compare_ilabels_for_qsort(const void *t1,const void *t2){
    unsigned long *trans1=(unsigned long *)t1;
    unsigned long ilabel1=trans1[0];
    
    unsigned long *trans2=(unsigned long *)t2;
    unsigned long ilabel2=trans2[0];
    
    if (ilabel1<ilabel2)
        return -1;
    else if (ilabel1>ilabel2)
        return 1;
        return 0;
    
    return 0;
}



FST_D_444 build_from_fst_o_fst_d_444(FST_O fst){
    
    int verbose=0;
    
    if (!fst || !nb_states_FST_O(fst))
        return NULL;
    
    FST_D_444 fst2=(FST_D_444)L_ALLOC(sizeof(struct _fst_d_444));
    memset(fst2,0,sizeof(struct _fst_d_444));
    
    FST_O_TRANS trans=init_trans_FST_O(fst);

    unsigned long nb_states=nb_states_FST_O(fst);
    
    // ### 1. Creates the table of size <nb_states>
    
    {
        fst2->nb_states=nb_states;
        
        fst2->state_types=(unsigned long *)L_ALLOC(sizeof(long)*nb_states);
        memset(fst2->state_types,0,sizeof(long)*nb_states);
        
        fst2->state_pos=(unsigned long *)L_ALLOC(sizeof(long)*nb_states);
        memset(fst2->state_pos,0,sizeof(long)*nb_states);
    }
    unsigned long *state_pos=fst2->state_pos;

    
    // ### 2. Computes the size of the max label
    
    {
        unsigned long max_ilabel=0;
        
        for(unsigned long i=0;i<nb_states;i++){
            
            for(int t=first_trans_FST_O(fst, trans, i);
                t;
                t=next_trans_FST_O(fst, trans)){
                
                if (max_ilabel<trans->ilabel)
                    max_ilabel=trans->ilabel;
                
            }
        }
        fst2->max_ilabel=max_ilabel;
    }
    
    // ### 3. Sorts states from the ones than have most transitions
    // down to the ones with least transition (except for the initial state
    // which should be first no matter what).
    // This is done to speed up the optimization
    
    unsigned long *state_trans_nb=(unsigned long *)L_ALLOC(sizeof(long)*2*nb_states);
    
    {
        for(unsigned long i=0;i<nb_states;i++){
            unsigned long nb_trans=0;
            for(int t=first_trans_FST_O(fst, trans, i);
                t;
                t=next_trans_FST_O(fst, trans)){
                nb_trans++;
            }
            state_trans_nb[2*i]=i;
            state_trans_nb[2*i+1]=nb_trans;
        }
        
        qsort(state_trans_nb,
              nb_states,
              sizeof(long)*2,
              &compare_state_trans_for_qsort);

    }
    
    // ### 4. Compression

    unsigned long current_starting_pos=1;
    
    DYN_LONG_ARRAY d_ilabel_table=init_with_size_dyn_long_array(nb_states*300);
    DYN_LONG_ARRAY d_olabel_table=init_with_size_dyn_long_array(nb_states*300);
    DYN_LONG_ARRAY d_arr_state_table=init_with_size_dyn_long_array(nb_states*300);
    DYN_LONG_ARRAY d_extra_trans_pos_table=init_with_size_dyn_long_array(nb_states*300);
    
    DYN_LONG_ARRAY d_extra_transitions_table=init_with_size_dyn_long_array(nb_states*300);
    
    // <d_local_transitions> stores the ilabel to that state
    // for efficiency reasons
    unsigned long nb_local_transitions;
    DYN_LONG_ARRAY d_local_transitions=init_with_size_dyn_long_array(1000);
    unsigned long *local_transitions=NULL;
    
    unsigned long max_pos_in_array=100;
    unsigned long label_pos=0;
    
    unsigned long current_extra_trans_pos=0;

    for(unsigned long i=0;i<nb_states;i++){
        
        unsigned long state_id=state_trans_nb[2*i];
        
        if (verbose){
            if (!(i%1000))
                fprintf(stdout, "compresses state %ld\n",i);
        }
        
        fst2->state_types[state_id]=state_type_FST_O(fst, state_id);
        
        // Puts all the ilabels into <d_local_transitions>
        // while ignoring repeating ilabels
        // This makes finding the next available location more efficient (which is the
        // expensive part).
        
        {
            nb_local_transitions=0;
            int needs_sorting=0;

            unsigned long previous_ilabel=LANG_MAX_LONG;
            for(int t=first_trans_FST_O(fst, trans, i);
                t;
                t=next_trans_FST_O(fst, trans)){
                if (trans->ilabel==previous_ilabel)
                    continue;
                set_at_dyn_long_array(d_local_transitions, nb_local_transitions++, trans->ilabel);
                if (previous_ilabel!=LANG_MAX_LONG & trans->ilabel<previous_ilabel)
                    needs_sorting=1;
                previous_ilabel=trans->ilabel;
            }
            
            local_transitions=(unsigned long *)d_local_transitions->array;
            // sorts the transitions if necessary (this allows to remove duplicates
            // because duplicates can be costly)
            if (needs_sorting){
                
                qsort(local_transitions,
                      nb_local_transitions,
                      sizeof(long),
                      &compare_ilabels_for_qsort);

                // Removes the remaining duplicates
                
                unsigned long l=0,m=0;
                previous_ilabel=LANG_MAX_LONG;
                while(m<nb_local_transitions){
                    if (local_transitions[m]==previous_ilabel)
                        continue;
                    local_transitions[l]=local_transitions[m];
                    previous_ilabel=local_transitions[l];
                    l++;m++;
                }
                nb_local_transitions=l;
            }
        }
        
        unsigned long pos=current_starting_pos;
        
        // Checks whether this position is available
        
        int found_position=0;
        
        while(!found_position){
            
            int pos_available=1;
            
            if (d_ilabel_table->array[pos]){
                pos_available=0;
                pos++;
                continue; // position not valie
            }
            
            for(unsigned long k=0;k<nb_local_transitions;k++){
                label_pos=pos + local_transitions[k] + 1;
                if (label_pos<d_ilabel_table->array_size && d_ilabel_table->array[label_pos]){
                    pos_available=0;
                    break;
                }
            }
            
            if (pos_available){
                found_position=1;
                break;
            }
            
            pos++;
        }
        
        state_pos[i]=pos;
        
        if (verbose){
            if (!(i%100)){
                fprintf(stdout, "DIRECT AUT: position for %ld: %ld\n",i,pos);
            }
        }

        // Builds the corresponding state
        
        
        // Below is to make sure that a state without transition has the same pos
        // as another state
        set_at_dyn_long_array(d_ilabel_table, pos, LANG_MAX_LONG);
        if (max_pos_in_array<pos)
            max_pos_in_array=pos;
        
        unsigned long previous_ilabel=LANG_MAX_LONG;
        unsigned long last_extra_trans_pos=0;
        int first_extra_trans_pos=0;
        
        for(int t=first_trans_FST_O(fst, trans, i);
            t;
            t=next_trans_FST_O(fst, trans)){
            
            unsigned long ac_pos=pos + trans->ilabel +1;
            
            if (trans->ilabel!=previous_ilabel){
                
                if (max_pos_in_array<ac_pos)
                    max_pos_in_array=ac_pos;
                
                set_at_dyn_long_array(d_ilabel_table, ac_pos, trans->ilabel+1);
                set_at_dyn_long_array(d_olabel_table, ac_pos, trans->olabel);
                set_at_dyn_long_array(d_arr_state_table, ac_pos, trans->arr_state);
                set_at_dyn_long_array(d_extra_trans_pos_table, ac_pos, 0);
                
                previous_ilabel=trans->ilabel;
                
                first_extra_trans_pos=1;
            }
            else{
                
                // Updates the number of extra transitions for this ilabel
                if (first_extra_trans_pos){
                    
                    set_at_dyn_long_array(d_extra_trans_pos_table, ac_pos, current_extra_trans_pos+1);
                    
                    // This is the first extra transition
                    last_extra_trans_pos=current_extra_trans_pos++;
                    set_at_dyn_long_array(d_extra_transitions_table,
                                          last_extra_trans_pos,
                                          1);
                    first_extra_trans_pos=0;
                }
                else{
                    (d_extra_transitions_table->array[last_extra_trans_pos])++;
                }
                set_at_dyn_long_array(d_extra_transitions_table,
                                      current_extra_trans_pos++,
                                      trans->olabel);
                set_at_dyn_long_array(d_extra_transitions_table,
                                      current_extra_trans_pos++,
                                      trans->arr_state);

            }
        }
    }
    
    // ### Builds up the final struture
    
    fst2->size_direct_access_table=max_pos_in_array+1;
    fst2->ilabel_table=(unsigned long *)L_ALLOC(sizeof(long)*fst2->size_direct_access_table);
    memcpy(fst2->ilabel_table,d_ilabel_table->array,sizeof(long)*fst2->size_direct_access_table);
    
    free_dyn_long_array(d_ilabel_table);
    
    fst2->olabel_table=(unsigned long *)L_ALLOC(sizeof(long)*fst2->size_direct_access_table);
    memcpy(fst2->olabel_table,d_olabel_table->array,sizeof(long)*fst2->size_direct_access_table);
    
    free_dyn_long_array(d_olabel_table);

    
    fst2->arr_state_table=(unsigned long *)L_ALLOC(sizeof(long)*fst2->size_direct_access_table);
    memcpy(fst2->arr_state_table,d_arr_state_table->array,sizeof(long)*fst2->size_direct_access_table);
    
    free_dyn_long_array(d_arr_state_table);
    
    fst2->extra_trans_pos_table=(unsigned long *)L_ALLOC(sizeof(long)*fst2->size_direct_access_table);
    memcpy(fst2->extra_trans_pos_table,d_extra_trans_pos_table->array,sizeof(long)*fst2->size_direct_access_table);

    free_dyn_long_array(d_extra_trans_pos_table);

    fst2->size_extra_transitions_table=current_extra_trans_pos;
    
    if (fst2->size_extra_transitions_table){
        fst2->extra_transitions_table=(unsigned long *)L_ALLOC(sizeof(long)*fst2->size_extra_transitions_table);
        memcpy(fst2->extra_transitions_table,d_extra_transitions_table->array,sizeof(long)*fst2->size_extra_transitions_table);
    }
    
    free_dyn_long_array(d_extra_transitions_table);
    
    // -- Sets the I_SYMB
    
    SYMB_O i_symb=i_symb_FST_O(fst);
    if (i_symb){
        fst2->i_symb=i_symb;
        fst2->i_symb->rcount++;
    }
    SYMB_O o_symb=o_symb_FST_O(fst);
    if (o_symb){
        fst2->o_symb=o_symb;
        fst2->o_symb->rcount++;
    }

    // -- Frees memory (what's left of it)
    
    free_fst_o_trans(trans);
    L_FREE(state_trans_nb);
    free_dyn_long_array(d_local_transitions);
    
    return fst2;
}

#pragma mark - Runtime

// Returns1 if there is a transition for
// <state_id> and <ilabel>
// If yes, sets p_olabel,p_arr,
int trans_for_ilabel_fst_d_444(FST_D_444 fst_d,
                               unsigned long state_id,
                               unsigned long ilabel,
                               unsigned long *p_olabel,
                               unsigned long *p_arr,
                               unsigned long *p_ilabel_pos){
    
    if (state_id>fst_d->nb_states)
        return 0;
    
    unsigned long ac=fst_d->state_pos[state_id]  + ilabel + 1;

    if (ac>=fst_d->size_direct_access_table)
        return 0;
    
    if (fst_d->ilabel_table[ ac ] != ilabel+1)
        return 0;
    
    *p_olabel=fst_d->olabel_table[ac];
    *p_arr=fst_d->arr_state_table[ac];
    *p_ilabel_pos=ac;
    
    return 1;
}

