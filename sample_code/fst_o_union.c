// fst_o_union.c

#include"lang.h"

#define NO_STATE_UNION 1111000000

int is_label_processed(unsigned long nb,DYN_LONG_ARRAY labels,unsigned long label){
    unsigned long i;
    if (!nb)
        return 0;
    
    for(i=0;i<nb;i++){
        if (label==labels->array[i])
            return 1;
    }
    
    return 0;
}


// ASSUMPTIONS:
// - Two deterministic automata
// WARNING: in case of ambiguity in the type of the terminal state
// it keeps the lowest (none null) value

FST_O union_with_ambiguity_fst_o(FST_O fst3_buff,
                                 FST_O fst1,
                                 FST_O fst2,
                                 int *p_had_ambiguity,
                                 ambiguity_resolution_type art,
                                 unsigned long (*ambiguous_state_resolution_callback)(unsigned long q1,
                                                                                      unsigned long q2,
                                                                                      void *callback_data),
                                 void *callback_data){
    int t;
    
    if (!fst1 || !nb_states_FST_O(fst1) || !fst2 || !nb_states_FST_O(fst2))
        return NULL;
    
    if (art==art_use_callback && !ambiguous_state_resolution_callback)
        return NULL;
    
    FST_O fst3=NULL;
    
    if (fst3_buff){
        fst3=fst3_buff;
    }
    else{
        fst3=fst_l_444_to_fst_o(init_fst_l_444());
    }
    
    reset_FST_O(fst3);
    
    // ### Builds the Initial State
    
    unsigned long q=0;
    unsigned long n=1; // assumes the initial state is created
    
    L_PAIRS pairs=init_l_pairs();
    
    
    add_pair_l_pairs(pairs, 0, 0);
    
    // adds the initial state
    add_state_FST_O(fst3);
    
    unsigned long q1=0;
    unsigned long q2=0;
    
    FST_O_TRANS trans1=init_trans_FST_O(fst1);
    FST_O_TRANS trans2=init_trans_FST_O(fst2);
    
    DYN_LONG_ARRAY processed_labels=init_dyn_long_array();
    unsigned long nb_processed_labels;
    
    while(q<n){
        
        get_pair_from_id_l_pairs(pairs, q, &q1, &q2);
        
        
        if (q1!=NO_STATE_UNION && q2!=NO_STATE_UNION){
            
            // ## a. Checks the state type
            
            unsigned long tp1=state_type_FST_O(fst1, q1);
            unsigned long tp2=state_type_FST_O(fst2, q2);
            
            if (tp1 && tp2){
                
                switch (art) {
                    case art_use_callback:
                    {
                        unsigned long tp=(*ambiguous_state_resolution_callback)(tp1,tp2,callback_data);
                        set_state_type_FST_O(fst3, q, tp);
                    }
                        break;
                    case art_first_has_priority:
                    {
                        unsigned long tp=(tp1<tp2) ? tp1 : tp2;
                        set_state_type_FST_O(fst3, q, tp);
                    }
                        break;
                        
                    default:
                    {
                        unsigned long tp=(tp1<tp2) ? tp1 : tp2;
                        set_state_type_FST_O(fst3, q, tp);

                    }
                        break;
                }
                
            }
            else if (tp1)
                set_state_type_FST_O(fst3, q, tp1);
            else if (tp2)
                set_state_type_FST_O(fst3, q, tp2);
            
            // ### b. Loops through the transitions of <fst1> while checking <fst2>
            
            nb_processed_labels=0;
            
            for( t=first_trans_FST_O(fst1, trans1,q1);
                t;
                t=next_trans_FST_O(fst1, trans1)){
                
                set_at_dyn_long_array(processed_labels, nb_processed_labels++, trans1->ilabel);
                
                if (first_trans_with_ilab_FST_O(fst2, trans2,q2, trans1->ilabel)){
                    
                    unsigned long qq_1=trans1->arr_state;
                    unsigned long qq_2=trans2->arr_state;
                    
                    unsigned long arr_state=0;
                    
                    if (!get_pair_id_l_pairs(pairs, qq_1, qq_2, &arr_state)){
                        
                        // first adds the missing state
                        
                        arr_state=add_state_FST_O(fst3);
                        add_pair_l_pairs(pairs, qq_1, qq_2);
                        
                        if (pairs->nb_pairs != nb_states_FST_O(fst3)){
                            // problem
                            fprintf(stderr,"[1] Should NOT be here\n");
                            return 0;
                        }
                    }
                    
                    add_trans_FST_O(NULL,fst3, q, arr_state, trans1->ilabel, trans1->olabel, trans1->weight);

                    
                }
                else{
                    
                    unsigned long qq_1=trans1->arr_state;
                    unsigned long qq_2=NO_STATE_UNION;
                    
                    unsigned long arr_state=0;
                    
                    if (!get_pair_id_l_pairs(pairs, qq_1, qq_2, &arr_state)){
                        
                        // first adds the missing state
                        
                        arr_state=add_state_FST_O(fst3);
                        add_pair_l_pairs(pairs, qq_1, qq_2);
                        
                        if (pairs->nb_pairs != nb_states_FST_O(fst3)){
                            // problem
                            fprintf(stderr,"[1] Should NOT be here\n");
                            return 0;
                        }
                    }
                    
                    add_trans_FST_O(NULL,fst3, q, arr_state, trans1->ilabel, trans1->olabel, trans1->weight);

                }
                
            }
            
            // ### c. Loops through the transitions of <fst2>
            
            for(t=first_trans_FST_O(fst2, trans2,q2);
                t;
                t=next_trans_FST_O(fst2, trans2)){
                
                // -- checks that this label has not been processed yets
                
                if (is_label_processed(nb_processed_labels, processed_labels, trans2->ilabel))
                    continue;
                
                unsigned long qq_1=NO_STATE_UNION;
                unsigned long qq_2=trans2->arr_state;
                
                unsigned long arr_state=0;
                
                if (!get_pair_id_l_pairs(pairs, qq_1, qq_2, &arr_state)){
                    
                    // first adds the missing state
                    
                    arr_state=add_state_FST_O(fst3);
                    add_pair_l_pairs(pairs, qq_1, qq_2);
                    
                    if (pairs->nb_pairs != nb_states_FST_O(fst3)){
                        // problem
                        fprintf(stderr,"[1] Should NOT be here\n");
                        return 0;
                    }
                }
                
                add_trans_FST_O(NULL,fst3, q, arr_state, trans2->ilabel, trans2->olabel, trans2->weight);


            }

            
        }
        else if (q1!=NO_STATE_UNION){
            
            set_state_type_FST_O(fst3, q, state_type_FST_O(fst1, q1));
            
            
            for( t=first_trans_FST_O(fst1, trans1,q1);
                t;
                t=next_trans_FST_O(fst1, trans1)){
                
                
                unsigned long qq_1=trans1->arr_state;
                unsigned long qq_2=NO_STATE_UNION;
                
                unsigned long arr_state=0;
                
                if (!get_pair_id_l_pairs(pairs, qq_1, qq_2, &arr_state)){
                    
                    // first adds the missing state
                    
                    arr_state=add_state_FST_O(fst3);
                    add_pair_l_pairs(pairs, qq_1, qq_2);
                    
                    if (pairs->nb_pairs != nb_states_FST_O(fst3)){
                        // problem
                        fprintf(stderr,"[1] Should NOT be here\n");
                        return 0;
                    }
                }
                
                add_trans_FST_O(NULL,fst3, q, arr_state, trans1->ilabel, trans1->olabel, trans1->weight);
                
            }
            
        }
        else{
            // q2!=NO_STATE_UNION
            
            set_state_type_FST_O(fst3, q, state_type_FST_O(fst2, q2));
            
            for(t=first_trans_FST_O(fst2, trans2,q2);
                t;
                t=next_trans_FST_O(fst2, trans2)){
                
                unsigned long qq_1=NO_STATE_UNION;
                unsigned long qq_2=trans2->arr_state;
                
                unsigned long arr_state=0;                
                
                if (!get_pair_id_l_pairs(pairs, qq_1, qq_2, &arr_state)){
                    
                    // first adds the missing state
                    
                    arr_state=add_state_FST_O(fst3);
                    add_pair_l_pairs(pairs, qq_1, qq_2);
                    
                    if (pairs->nb_pairs != nb_states_FST_O(fst3)){
                        // problem
                        fprintf(stderr,"[1] Should NOT be here\n");
                        return 0;
                    }
                }
                
                add_trans_FST_O(NULL,fst3, q, arr_state, trans2->ilabel, trans2->olabel, trans1->weight);
            }
            
        }

        n=nb_states_FST_O(fst3);
        q++;

    }
    
    free_l_pairs(pairs);

    free_dyn_long_array(processed_labels);
    
    free_fst_o_trans(trans1);
    free_fst_o_trans(trans2);
    
    return fst3;
}

FST_O union_fst_o(FST_O fst3_buff,FST_O fst1,FST_O fst2){
    
    int had_ambiguity;
    
    return union_with_ambiguity_fst_o(fst3_buff,
                                      fst1,
                                      fst2,
                                      &had_ambiguity,
                                      art_first_has_priority,
                                      NULL,
                                      NULL);

}

FST_O union_multiple_fst_o(FST_O *fsts,unsigned long nb_fsts){
    
    if (!fsts || nb_fsts<=1)
        return NULL;
    
    FST_O current_union=fsts[0];
    
    for(unsigned long i=1;i<nb_fsts;i++){
        FST_O new_fst=union_fst_o(NULL, current_union, fsts[i]);
        
        if (current_union!=fsts[0]){
            free_fst_o(current_union);
        }
        
        current_union=new_fst;
    }
    
    return current_union;
}
