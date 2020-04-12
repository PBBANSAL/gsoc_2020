/*
 * synonyms_impl.c: Implementation of synonyms functions
 *
 * Before modifying this file read README.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "synonyms_impl.h"

struct _Word {
    char* word;
    struct _Word* synonym;
    struct _Word* next_syn; 
    struct _Word* next; 
};

typedef struct _Word Word;

#define HASH_SIZE 100

int hash_function(const char* word) {
    unsigned long hash = 7;
    int i = 0;
    for (i = 0; i<strlen(word); i++) {
        hash = hash*31 + word[i];
    }
    return (int)(hash % HASH_SIZE);
}

struct _Synonyms {
    /* Implement me */
    // HashTable implementation.
    Word *word_ht[HASH_SIZE];
};

Word* init_word(const char* w) {
    Word* word = malloc(sizeof(Word));
    memset(word, 0, sizeof(Word));
    word->word = malloc(strlen(w)+1);
    memset(word->word, 0, strlen(w)+1);
    strncpy(word->word, w, strlen(w));
    word->word[strlen(w)] = '\0';
    word->synonym = NULL;
    word->next_syn = NULL;
    return word;
}

/**
 * synonyms_init:
 *
 * Returns: an instance of synonyms dictionary, or
 *          NULL on error (with errno set properly).
 */
Synonyms *
synonyms_init(void)
{
    /* Implement me */
	Synonyms* s = malloc(sizeof(Synonyms));
    memset(s, 0, sizeof(Synonyms));
    return s;
}


/**
 * synonyms_free:
 * @s: instance of synonyms dictionary
 *
 * Frees previously allocated dictionary. If @s is NULL then this
 * is NO-OP.
 */
void
synonyms_free(Synonyms *s)
{
    /* Implement me */
    int i=0;
    for(i=0; i<HASH_SIZE; i++) {
        Word* w = s->word_ht[i];
	if (w) {
        while (w->next) {
            Word* prev = w;
            w = w->next;
            free(prev);
       	}
        free(w);
	}
    }

    free(s);
    s = NULL;
}

// HashTable functions.

Word* exists_in_ht(Synonyms *s, const char* word) {
    int ind = hash_function(word);

    Word* w = s->word_ht[ind];

    while(w) {
        if(strcmp(w->word, word) == 0) {
            return w;
        }
        w = w->next;
    }

    return NULL;
}

void insert_in_ht(Synonyms *s, Word* word) {
    int ind = hash_function(word->word);

    Word* w = s->word_ht[ind];
    if(!w) {
        s->word_ht[ind] = word;
        return;
    }

    while(w->next) {
        w = w->next;
    }

    w->next = word;
}

/**
 * add_synonym
 * @s: Synonyms dictionary
 * @w: Word to which a new synonym is being added.
 * @sn: New synonym being added to w.
 *
 * For a given @w, adds @sn as a synonym.
 */
void add_synonym(Synonyms *s, Word* w, Word* sn) {
    if(!w->synonym) {
        w->synonym = sn;
	return;
    }

    Word* syns = w->synonym;
    while(syns->next_syn) {
        syns = syns->next_syn;
    }

    syns->next_syn = sn;
}

/**
 * synonyms_define:
 * @s: instance of synonyms dictionary
 * @word: a word to add to the dictionary
 * @args: a list of synonyms
 *
 * For given @word, add it to the dictionary and define its synonyms. If the
 * @word already exists in the dictionary then just extend its list of
 * synonyms.
 *
 * Returns 0 on success, -1 otherwise.
 */
int
synonyms_define(Synonyms *s,
                const char *word, ...)
{
    Word* first = NULL;
    if(!(first = exists_in_ht(s, word))) {
        first = init_word(word);
        if (!first) {
            return -1;
        }
        insert_in_ht(s, first);
    }

    // Iterate over all __VA_ARGS__.
    Word* w = NULL;
    va_list ap;

    va_start(ap, word);

    char* synonym = va_arg(ap, char*);
    while(synonym) {
        w = NULL;
        if(!(w = exists_in_ht(s, synonym))) {
                w = init_word(synonym);
                if (!w) {
                    return -1;
                }
                insert_in_ht(s, w);
        }

        add_synonym(s, first, w);
        add_synonym(s, w, first);
    	synonym = va_arg(ap, char*);
    }
    va_end(ap);

    return 0;
}

// Add a new synonym to the list of synonyms to be later
// for creating a string list.
int add_to_list(Word** list, int num_words, Word* syn) {
    Word* w = NULL;

    if(*list) {
        w = *list;
        while(w->next) {
            if(strcmp(w->word, syn->word) == 0) {
                return num_words;
            }
            w = w->next;
        }

        if(w && strcmp(w->word, syn->word) == 0) {
            return num_words;
        }
    }
    
    Word* new_word = malloc(sizeof(Word));
    memcpy(new_word, syn, sizeof(Word));
    new_word->next_syn = NULL;
    new_word->next = NULL;

    if(!w) {
        *list = new_word;
    } else {
        w->next = new_word;
    }
    return num_words+1;
}

// Find synonyms for the given word.
void find_synonyms(Synonyms *s, Word** list, int *num_words, Word* word, Word* orig) {
    Word* synonyms = word->synonym;
    while(synonyms) {
	if (synonyms != orig && strcmp(synonyms->word, orig->word) != 0) {
            int old_num = *num_words;
	    *num_words = add_to_list(list, *num_words, synonyms);
	    if(*num_words > old_num) {
            	find_synonyms(s, list, num_words, synonyms, orig);
	    }
        }	
        synonyms = synonyms->next_syn;
    }
}


/*
 * is_synonym:
 * @s: instance of synonyms dictionary
 * @w1: a word
 * @w2: a word
 *
 * Checks whether @w1 is defined synonym of @w2 (or vice versa).
 */
bool
is_synonym(Synonyms *s,
           const char *w1,
           const char *w2)
{
    /* Implement me */
    Word* list = NULL;
    int num_words = 0;

    Word* w = NULL;
    if ((w = exists_in_ht(s, w1))) {
        Word* synonyms = w->synonym;
        while(synonyms) {
            num_words = add_to_list(&list, num_words, synonyms);
            find_synonyms(s, &list, &num_words, synonyms, w);
            synonyms = synonyms->next_syn;
        }
    }

    while(list) {
        if(list->word && strcmp(list->word, w2) == 0) {
            return true;
        }
	list = list->next;
    }

    return false;
}

char** get_str_list(Word* list, int num_words) {
    char** str_list = malloc(sizeof(char*)*(num_words+1));
    
    int i=0;
    for(i=0; i<num_words; ++i) {
        str_list[i] = malloc(strlen(list->word)+1);
        memcpy(str_list[i], list->word, strlen(list->word)+1);
        list = list->next;
    }
    str_list[num_words] = NULL;
 
    return str_list;
}

/**
 * synonyms_get:
 * @s: instance of synonyms dictionary
 * @word: a word
 *
 * Returns: a string list of defined synonyms for @word, or
 *          NULL if no synonym was defined or an error occurred.
 */
char **
synonyms_get(Synonyms *s,
             const char *word)
{
    Word* list = NULL;
    int num_words = 0;

    /* Implement me */
    Word* w = NULL;
    if ((w = exists_in_ht(s, word))) {
        Word* synonyms = w->synonym;
        while(synonyms) {
            num_words = add_to_list(&list, num_words, synonyms);
            find_synonyms(s, &list, &num_words, synonyms, w);
            synonyms = synonyms->next_syn;
        }
    }

    return get_str_list(list, num_words);
}
