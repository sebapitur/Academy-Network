#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "publications.h"
#include "Hashtable.h"
#include "Queue.h"
#include "LinkedList.h"

struct paper {
    char *title;
    int year;
    int num_refs;
    int64_t *references;
    int num_fields;
    char **fields;
    int visit;
   	int dist;
};

struct author {
    char name[1000];
    struct LinkedList *venues;
	struct LinkedList *id_papers;
	struct LinkedList *institutions;
	int visit;
	int dist;
};

struct publications_data {
    /* TODO: add whichever data you need here */
    struct Hashtable *ht_papers;
    struct Hashtable *ht_citations;
    struct Hashtable *ht_venues;
    struct Hashtable *ht_authors;
    struct Hashtable *ht_collabarations;
    struct Hashtable *ht_institutions;
    int *years;
};

void free_paper_ht(void *info) {
    int i;

    free(((struct paper*)info)->title);
    free(((struct paper*)info)->references);
    for(i = 0; i < ((struct paper*)info)->num_fields; i++)
    	free(((struct paper*)info)->fields[i]);
    free(((struct paper*)info)->fields);
    free(((struct paper*)info));
}

void free_list_ht(void *info) {
	free_list_improved((struct LinkedList*)info);
}

void free_author_ht(void *info) {
	free_list_improved(((struct author*)info)->id_papers);
	free_list_improved(((struct author*)info)->institutions);
    free_list_improved(((struct author*)info)->venues);
	free((struct author*)info);
}

void reset_visit(PublData* data, struct Queue *v) {
	int index;
	struct Node *curr;
	struct paper *info;

	while(!is_empty_q(v)) {
        index = *(int*)front(v);
        curr = data->ht_papers->buckets[index].head;
        while(curr) {
            info = (struct paper*)((struct info*)curr->data)->value;
            info->visit = 0;
            info->dist = 0;
            curr = curr->next;
        }
        dequeue_improved(v);
    }
}

void reset_visit_authors(PublData* data, struct Queue *v) {
	int index;
	struct Node *curr;
	struct author *info;

	while(!is_empty_q(v)) {
        index = *(int*)front(v);
        curr = data->ht_authors->buckets[index].head;
        while(curr) {
            info = (struct author*)((struct info*)curr->data)->value;
            info->visit = 0;
            info->dist = 0;
            curr = curr->next;
        }
        dequeue_improved(v);
    }
}

PublData* init_publ_data(void) {
    /* TODO: implement init_publ_data */
    PublData *data;
    data = malloc(sizeof(PublData));

    data->ht_papers = malloc(sizeof(struct Hashtable));
    init_ht(data->ht_papers, HMAX, hash_function_int, compare_function_ints);
    
    data->ht_citations = malloc(sizeof(struct Hashtable));
    init_ht(data->ht_citations, HMAX, hash_function_int, compare_function_ints);
    
    data->ht_venues = malloc(sizeof(struct Hashtable));
    init_ht(data->ht_venues, HMAX, hash_function_string, compare_function_strings);

    data->ht_authors = malloc(sizeof(struct Hashtable));
    init_ht(data->ht_authors, HMAX, hash_function_int, compare_function_ints);

    data->ht_collabarations = malloc(sizeof(struct Hashtable));
    init_ht(data->ht_collabarations, HMAX, hash_function_int, compare_function_ints);

    data->ht_institutions = malloc(sizeof(struct Hashtable));
    init_ht(data->ht_institutions, HMAX, hash_function_string, compare_function_strings);

    data->years = calloc(MAX_YEAR, sizeof(int));
    return data;
}

void destroy_publ_data(PublData* data) {
    /* TODO: implement destroy_publ_data */
    free_ht(data->ht_papers, free_paper_ht);
    free_ht(data->ht_citations, free_list_ht);
    free_ht(data->ht_venues, free_list_ht);
    free_ht(data->ht_authors, free_author_ht);
    free_ht(data->ht_collabarations, free_list_ht);
    free_ht(data->ht_institutions, free_list_ht);
    free(data->years);
    free(data);
}

void add_paper(PublData* data, const char* title, const char* venue,
    const int year, const char** author_names, const int64_t* author_ids,
    const char** institutions, const int num_authors, const char** fields,
    const int num_fields, const int64_t id, const int64_t* references,
    const int num_refs) {
    /* TODO: implement add_paper */
    int i, j;
    struct paper *info;
    struct author *author;
    int64_t key, id_paper = id, id_author, id_author_insert;
    struct LinkedList *list, *list_ids, *list_authors;
    char *venue_key = venue, *institution, venue_copy[1000];

    key = id;
    info = malloc(sizeof(struct paper));
    info->title = malloc((strlen(title) + 1) * sizeof(char));
    memcpy(info->title, title, strlen(title) + 1);
    info->year = year;
    info->num_refs = num_refs;
    info->references = malloc(num_refs * sizeof(int64_t));
    info->num_fields = num_fields;
    info->fields = malloc(num_fields * sizeof(char*));
    for(i = 0; i < num_fields; i++) {
    	info->fields[i] = malloc((strlen(fields[i]) + 1) * sizeof(char));
    	memcpy(info->fields[i], fields[i], strlen(fields[i]) + 1);
    }
    info->visit = 0;
    info->dist = 0;


    for(i = 0; i < num_refs; i++) {
        info->references[i] = references[i];
        list_ids = get(data->ht_citations, &info->references[i]);
        if(list_ids) {
            add_last_improved(list_ids, &id_paper, sizeof(int64_t));
        } else {
            list_ids = malloc(sizeof(struct LinkedList));
            init_list(list_ids);
            add_last_improved(list_ids, &id_paper, sizeof(int64_t));
            put(data->ht_citations, &info->references[i], sizeof(int64_t), list_ids);
        }
    }


    put(data->ht_papers, &key, sizeof(int64_t), info);


    list = get(data->ht_venues, venue_key);
    if(list) {
    	add_last_improved(list, &id_paper, sizeof(int64_t));
    } else {
    	list = malloc(sizeof(struct LinkedList));
    	init_list(list);
    	add_last_improved(list, &id_paper, sizeof(int64_t));
    	put(data->ht_venues, venue_key, strlen(venue_key) + 1, list);
    }


    for(i = 0; i < num_authors; i++) {
    	id_author = author_ids[i];
    	institution = institutions[i];
    	author = get(data->ht_authors, &id_author);
    	if(author) {
    		add_last_improved(author->id_papers, &id_paper, sizeof(int64_t));
    		add_last_improved(author->institutions, institution, (strlen(institution) + 1) * sizeof(char));
            memset(venue_copy, 0, 1000);
            strcpy(venue_copy, venue);
            add_last_improved(author->venues, venue_copy, (strlen(venue_copy) + 1) * sizeof(char));
    	} else {
    		author = malloc(sizeof(struct author));
    		author->id_papers = malloc(sizeof(struct LinkedList));
    		init_list(author->id_papers);
    		add_last_improved(author->id_papers, &id_paper, sizeof(int64_t));
    		author->institutions = malloc(sizeof(struct LinkedList));
    		init_list(author->institutions);
    		add_last_improved(author->institutions, institution, (strlen(institution) + 1) * sizeof(char));
            author->venues = malloc(sizeof(struct LinkedList));
            init_list(author->venues);
            memset(venue_copy, 0, 1000);
            strcpy(venue_copy, venue);
            // printf("%s\n", venue_copy);
            add_last_improved(author->venues, venue_copy, (strlen(venue_copy) + 1) * sizeof(char));
    		author->visit = 0;
    		author->dist = 0;
            strcpy(author->name, author_names[i]);
    		put(data->ht_authors, &id_author, sizeof(int64_t), author);
    	}


    	list_authors = get(data->ht_collabarations, &id_author);
    	if(list_authors) {
    		for(j = 0; j < num_authors; j++) {
    			id_author_insert = author_ids[j];
    			add_last_improved(list_authors, &id_author_insert, sizeof(int64_t));
    		}
    	} else {
    		list_authors = malloc(sizeof(struct LinkedList));
    		init_list(list_authors);
    		for(j = 0; j < num_authors; j++) {
    			id_author_insert = author_ids[j];
    			add_last_improved(list_authors, &id_author_insert, sizeof(int64_t));
    		}
    		put(data->ht_collabarations, &id_author, sizeof(int64_t), list_authors);
    	}


    	list_authors = get(data->ht_institutions, institution);
    	if(list_authors) {
			id_author_insert = author_ids[i];
			add_last_improved(list_authors, &id_author_insert, sizeof(int64_t));
    	} else {
    		list_authors = malloc(sizeof(struct LinkedList));
    		init_list(list_authors);
			id_author_insert = author_ids[i];
			add_last_improved(list_authors, &id_author_insert, sizeof(int64_t));
    		put(data->ht_institutions, institution, strlen(institution) + 1, list_authors);
    	}
    }


    data->years[year]++;
}

char* get_oldest_influence(PublData* data, const int64_t id_paper) {
    /* TODO: implement get_oldest_influence */
    struct Queue *q, *v;
    int64_t id = id_paper, min_id;
    struct paper *info, *info_top;
    struct LinkedList *list;
    int min_year = MAX_YEAR, index, i, max_num_cits = 0, cits;
    char *title = "None";

    info = get(data->ht_papers, &id);
    if(!info || !info->num_refs)
        return title;

    q = malloc(sizeof(struct Queue));
    init_q(q);
    info->visit = 1;
    enqueue(q, info);

    v = malloc(sizeof(struct Queue));
    index = data->ht_papers->hash_function(&id) % data->ht_papers->hmax;
    init_q(v);
    enqueue_improved(v, &index, sizeof(int));

    while(!is_empty_q(q)) {
        info_top = (struct paper*)front(q);
        dequeue(q);
        for(i = 0; i < info_top->num_refs; i++) {
            id = info_top->references[i];
            info = get(data->ht_papers, &id);
            if(info && info->visit == 0) {
                info->visit = 1;
                index = data->ht_papers->hash_function(&id) % data->ht_papers->hmax;
                enqueue(q, info);
                enqueue_improved(v, &index, sizeof(int));
				list = get(data->ht_citations, &id);
				cits = get_size(list);
                if(min_year > info->year) {
                    title = info->title;
                    min_year = info->year;
                    max_num_cits = cits;
                    min_id = id;
                }
                else if(min_year == info->year) {
                    if(max_num_cits < cits) {
                        title = info->title;
                        max_num_cits = cits;
                        min_id = id;
                    }
                    else if(max_num_cits == cits) {
                        if(min_id > id) {
                            title = info->title;
                            min_id = id;
                        }
                    }
                }
            }
        }
    }

    reset_visit(data, v);
    purge_q_improved(v);
    purge_q(q);

    return title;
}

float get_venue_impact_factor(PublData* data, const char* venue) {
    /* TODO: implement get_venue_impact_factor */
	struct LinkedList *list;
	struct Node *curr;
	int num, sum = 0;
	char *venue_key = venue;
	int64_t id;
	float factor;

	list = get(data->ht_venues, venue_key);
	num = get_size(list);
	curr = list->head;
	while(curr) {
		id = *(int*)curr->data;
		list = get(data->ht_citations, &id);
		if(list)
			sum += get_size(list);
		curr = curr->next;
	}
	factor = (float) sum / num;
    
    return factor;
}

int get_number_of_influenced_papers(PublData* data, const int64_t id_paper,
    const int distance) {
    /* TODO: implement get_number_of_influenced_papers */
    int64_t id = id_paper, id_top;
    struct LinkedList *list;
    struct Queue *q, *v;
    struct Node *curr;
    struct paper *info, *info_top;
    int num_infl = 0, index;

    info = get(data->ht_papers, &id);
    if(!info)
    	return num_infl;

    q = malloc(sizeof(struct Queue));
    init_q(q);
    info->visit = 1;
    enqueue_improved(q, &id, sizeof(int64_t));

    v = malloc(sizeof(struct Queue));
    index = data->ht_papers->hash_function(&id) % data->ht_papers->hmax;
    init_q(v);
    enqueue_improved(v, &index, sizeof(int));

    while(!is_empty_q(q)) {
        id_top = *(int64_t*)front(q);
        info_top = get(data->ht_papers, &id_top);
        dequeue_improved(q);

        if(info_top->dist > distance)
        	break;

        list = get(data->ht_citations, &id_top);
        if(list) {
	        curr = list->head;
	        while(curr) {
                id = *(int64_t*)curr->data;
                info = get(data->ht_papers, &id);
                if(info->visit == 0) {
                    info->visit = 1;
                    info->dist = info_top->dist + 1;
                    if(info->dist <= distance)
       					num_infl++;
                    index = data->ht_papers->hash_function(&id) % data->ht_papers->hmax;
                    enqueue_improved(q, &id, sizeof(int64_t));
                    enqueue_improved(v, &index, sizeof(int));
                }
                curr = curr->next;
            }
       	}
    }

    reset_visit(data, v);
    purge_q_improved(v);
    purge_q_improved(q);

    return num_infl;
}

int get_erdos_distance(PublData* data, const int64_t id1, const int64_t id2) {
    /* TODO: implement get_erdos_distance */
	int64_t id = id1, id_top;
	struct author *author, *author_top;
	struct Queue *q, *v;
	struct LinkedList *list;
	struct Node *curr;
	int index, min_dist = MAX_DIST, found = 0;

	author = get(data->ht_authors, &id);
	if(!author)
		return -1;

	q = malloc(sizeof(struct Queue));
    init_q(q);
    author->visit = 1;
    enqueue_improved(q, &id, sizeof(int64_t));

    v = malloc(sizeof(struct Queue));
    index = data->ht_authors->hash_function(&id) % data->ht_authors->hmax;
    init_q(v);
    enqueue_improved(v, &index, sizeof(int));

    while(!found && !is_empty_q(q)) {
        id_top = *(int64_t*)front(q);
        author_top = get(data->ht_authors, &id_top);
        dequeue_improved(q);

        list = get(data->ht_collabarations, &id_top);
        if(list) {
	        curr = list->head;
	        while(curr) {
                id = *(int64_t*)curr->data;
                // printf("%lld   %lld\n", id, id2);
                author = get(data->ht_authors, &id);

                if(author->visit == 0) {
                    // printf("%s\n","OK");
                    author->visit = 1;
                    author->dist = author_top->dist + 1;
                    index = data->ht_authors->hash_function(&id) % data->ht_authors->hmax;
                    enqueue_improved(q, &id, sizeof(int64_t));
                    enqueue_improved(v, &index, sizeof(int));
                    if(id == id2) {
                        // printf("%s\n","OK");
                    	found = 1;
                    	min_dist = author->dist;
                    	break;
                    }
                }
                curr = curr->next;
            }
       	}
    }

    reset_visit_authors(data, v);
    purge_q_improved(v);
    purge_q_improved(q);

    if(min_dist != MAX_DIST)
    	return min_dist;
    return -1;
}

char** get_most_cited_papers_by_field(PublData* data, const char* field,
    int* num_papers) {
    /* TODO: implement get_most_cited_papers_by_field */

    return NULL;
}

int get_number_of_papers_between_dates(PublData* data, const int early_date,
    const int late_date) {
    /* TODO: implement get_number_of_papers_between_dates */
    int i, num_papers = 0;

    for(i = early_date; i <= late_date; i++)
    	num_papers += data->years[i];

    return num_papers;
}

int get_number_of_authors_with_field(PublData* data, const char* institution,
    const char* field) {
    /* TODO: implement get_number_of_authors_with_field */
	int num_authors = 0, index, found, i;
	char *inst = institution;
	int64_t id_author, id_paper;
	struct author* author;
	struct paper *paper;
	struct LinkedList *list_authors, *list_papers, *list_institutions;
	struct Node *curr_author, *curr_paper, *curr_institution;
	struct Queue *v;

	v = malloc(sizeof(struct Queue));
	init_q(v);

	list_authors = get(data->ht_institutions, inst);
	if(list_authors) {
		curr_author = list_authors->head;
		while(curr_author) {
			id_author = *(int64_t*)curr_author->data;
			author = get(data->ht_authors, &id_author);
			if(author && !author->visit) {
				author->visit = 1;
				index = data->ht_authors->hash_function(&id_author) % data->ht_authors->hmax;
                enqueue_improved(v, &index, sizeof(int));
                list_papers = author->id_papers;
                list_institutions = author->institutions;
                if(list_papers) {
                	curr_paper = list_papers->head;
                	curr_institution = list_institutions->head;
                	found = 0;

                	while(!found && curr_paper) {	
                		if(curr_institution && !strcmp((char*)curr_institution->data, inst)) {
	                		id_paper = *(int64_t*)curr_paper->data;
	                		paper = get(data->ht_papers, &id_paper);
	                		if(paper) {
	                			for(i = 0; i < paper->num_fields; i++) {
	                				if(!strcmp(paper->fields[i], field)) {
	                					num_authors++;
	                					found = 1;
	                					break;
	                				}
	                			}
	                		}
                		}
                		curr_paper = curr_paper->next;
                		curr_institution = curr_institution->next;
                	}
                }
			}
			curr_author = curr_author->next;
		}
	}

    reset_visit_authors(data, v);
    purge_q_improved(v);

    return num_authors;
}

int* get_histogram_of_citations(PublData* data, const int64_t id_author,
    int* num_years) {
    /* TODO: implement get_histogram_of_citations */
	int *histogram, year;
	int64_t id = id_author, id_paper;
	struct author *author;
	struct LinkedList *id_papers, *list_citations;
	struct Node *curr_paper;
	struct paper *paper;

	*num_years = 0;
	author = get(data->ht_authors, &id);
	id_papers = author->id_papers;

	if(id_papers) {
		curr_paper = id_papers->head;
		while(curr_paper) {
			id_paper = *(int64_t*)curr_paper->data;
			paper = get(data->ht_papers, &id_paper);
			year = paper->year;
			if(CURRENT_YEAR - *num_years + 1 > year) {
				*num_years = CURRENT_YEAR - year + 1;
			}
			curr_paper = curr_paper->next;
		}
	}

    histogram = calloc(*num_years, sizeof(int));
	if(*num_years) {
		curr_paper = id_papers->head;
		while(curr_paper) {
			id_paper = *(int64_t*)curr_paper->data;
			paper = get(data->ht_papers, &id_paper);
			year = paper->year;
			list_citations = get(data->ht_citations, &id_paper);
			if(list_citations) {
				histogram[CURRENT_YEAR - year] += get_size(list_citations);
			}
			curr_paper = curr_paper->next;
		}
	}

    return histogram;
}

char** get_reading_order(PublData* data, const int64_t id_paper,
    const int distance, int* num_papers) {
    /* TODO: implement get_reading_order */

    *num_papers = 0;

    return NULL;
}

char* find_best_coordinator(PublData* data, const int64_t id_author) {
    /* TODO: implement find_best_coordinator */
    int dim, i, max_score = -1, score, erdos_dist, no_cit;
    int64_t id_cacat, idmin = 100000000000;
    float fact;
    struct LinkedList *list, *list_v, *list_cit;
    struct Node *curr, *curr_list, *curr_list_v;
    char curr_name[1000], max_name[1000], venue[1000], *p_max;
    dim = data->ht_authors->size;
    // struct author a;
    // a = *(struct author *)get(data->ht_authors, &id_author);
    // printf("%s\n", a.name);
    // printf("%d\n",get_erdos_distance(data, 2148035700,2095652754));
    // printf("%d\n",get_erdos_distance(data, 2148035700,2095652754));
    // return NULL;
    for(i = 0; i < data->ht_authors->hmax; i++) {
        // printf("OK\n");
        curr = data->ht_authors->buckets[i].head;
        while(curr) {
            // printf("OK\n");
            memset(curr_name, 0, 1000);
            strcpy(curr_name, ((struct author *)((struct info*)(curr->data))->value)->name);
             // printf("%s ", curr_name);
             // printf("%lld\n",(const int64_t)(*(int64_t *)(((struct info*)(curr->data))->key)));
             // if((const int64_t)(*(int64_t *)(((struct info*)(curr->data))->key)) == 2095652754)
             //    printf("OK ");
            id_cacat  = (const int64_t)(*(int64_t *)(((struct info*)(curr->data))->key));
            erdos_dist = get_erdos_distance(data,(const int64_t) id_cacat, id_author);
            // printf("%d\n", erdos_dist);
            if((erdos_dist >= 1 && erdos_dist <= 5)) {
                // printf("%s\n", curr_name);
                // printf("%d\n", erdos_dist);
                list = ((struct author *)((struct info*)(curr->data))->value)->id_papers;
                list_v = ((struct author *)((struct info*)(curr->data))->value)->venues;
                curr_list = list->head;
                curr_list_v = list_v->head;
                score = 0;
                while(curr_list) {
                    // printf("%s\n", (char *)(curr_list_v->data));
                    fact = get_venue_impact_factor(data, (char *)(curr_list_v->data));
                    // printf("%lld\n", *((int64_t *)(curr_list)->data));
                    list_cit = get(data->ht_citations, ((int64_t *)(curr_list)->data));
                    // printf("OK\n");
                    if(list_cit != NULL) {
                        no_cit = list_cit->size;
                        // printf("OK\n");
                        score += (float)fact * (no_cit);
                    } 
                    curr_list = curr_list->next;
                    curr_list_v = curr_list_v->next;
                }
                // printf("%d\n", score);
                score = score * exp(-erdos_dist);
                if(score > max_score ) {
                    memset(max_name, 0, 1000);
                    strcpy(max_name, curr_name);
                    // printf("%s\n", max_name);
                    max_score = score;
                }
                if(score == max_score && idmin > id_cacat) {
                    memset(max_name, 0, 1000);
                    strcpy(max_name, curr_name);
                    // printf("%s\n", max_name);
                    max_score = score;
                    idmin = id_cacat;
                }

            }
            curr = curr->next;        
        }
    }
    if(max_score != -1){
        // printf("%d\n", max_score);
        p_max = max_name;
        return p_max;
    } else {
        return "None";
    }
}
