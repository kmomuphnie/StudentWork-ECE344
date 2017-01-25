#ifndef _WC_H_
#define _WC_H_

/* DO NOT CHANGE THIS FILE */

/* Forward declaration of structure for the function declarations below. */
struct wc;

/* Initialize data structure, returning pointer to it. */
struct wc *wc_init(void);

/* Insert word in wc, so that you can track it. Return 1 on success and 0 on
 * error (e.g., out of memory). */
int wc_insert_word(struct wc *wc, char *word);

/* wc_output produces output, consisting of unique words that have been inserted
 * in wc, and a count of the number of times each word has been seen. The output
 * should be in the format shown below.
------------------
word1:5
word2:10
word3:30
word4:1
------------------
  * The words do not have to be sorted in any order. Do not add any extra
  * whitespace.
 */
void wc_output(struct wc *wc);

#endif /* _WC_H_ */
