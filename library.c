/*
 * LIBRARY MANAGEMENT SYSTEM  (Data Structures Lab Project)
 * ---------------------------------------------------------
 * Data structures used and where:
 *   1. Binary Search Tree   -> catalogue of books keyed by Book ID
 *                              (insert, search, delete, inorder traversal)
 *   2. Hash Table (chaining)-> fast member lookup by Member ID
 *   3. Singly Linked List   -> books currently issued to each member
 *   4. Queue (linked)       -> FIFO waitlist / reservations per book
 *   5. Stack (linked)       -> transaction history + UNDO (LIFO)
 *   6. Merge Sort (array)   -> display catalogue sorted by title
 *   7. Linear search / traversal -> keyword search in title/author
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>

#define MAX_LOAN   3     /* max books a member can hold */
#define HASH_SIZE  11

/* ------------------------- QUEUE (waitlist) ------------------------- */
typedef struct QNode {
    int memberId;
    struct QNode *next;
} QNode;

typedef struct {
    QNode *front, *rear;
    int size;
} Queue;

void qInit(Queue *q) { q->front = q->rear = NULL; q->size = 0; }

void enqueue(Queue *q, int memberId) {
    QNode *n = malloc(sizeof(QNode));
    n->memberId = memberId; n->next = NULL;
    if (q->rear) q->rear->next = n; else q->front = n;
    q->rear = n; q->size++;
}

int dequeue(Queue *q) {
    if (!q->front) return -1;
    QNode *n = q->front;
    int id = n->memberId;
    q->front = n->next;
    if (!q->front) q->rear = NULL;
    free(n); q->size--;
    return id;
}

int inQueue(Queue *q, int memberId) {
    for (QNode *n = q->front; n; n = n->next)
        if (n->memberId == memberId) return 1;
    return 0;
}

void qClear(Queue *q) { while (q->front) dequeue(q); }

/* ------------------------- BST (books) ------------------------- */
typedef struct Book {
    int id;
    char title[64];
    char author[64];
    int total, avail;
    Queue waitlist;
    struct Book *left, *right;
} Book;

Book *catalogue = NULL;
int bookCount = 0;

Book *newBook(int id, const char *title, const char *author, int copies) {
    Book *b = malloc(sizeof(Book));
    b->id = id;
    strncpy(b->title, title, 63);   b->title[63] = '\0';
    strncpy(b->author, author, 63); b->author[63] = '\0';
    b->total = b->avail = copies;
    qInit(&b->waitlist);
    b->left = b->right = NULL;
    return b;
}

Book *bstSearch(Book *root, int id) {
    while (root && root->id != id)
        root = (id < root->id) ? root->left : root->right;
    return root;
}

Book *bstInsert(Book *root, Book *nb, int *ok) {
    if (!root) { *ok = 1; return nb; }
    if (nb->id < root->id)      root->left  = bstInsert(root->left, nb, ok);
    else if (nb->id > root->id) root->right = bstInsert(root->right, nb, ok);
    else *ok = 0;               /* duplicate id */
    return root;
}

Book *bstMin(Book *n) { while (n->left) n = n->left; return n; }

Book *bstDelete(Book *root, int id) {
    if (!root) return NULL;
    if (id < root->id)      root->left  = bstDelete(root->left, id);
    else if (id > root->id) root->right = bstDelete(root->right, id);
    else {
        if (!root->left || !root->right) {
            Book *child = root->left ? root->left : root->right;
            qClear(&root->waitlist);
            free(root);
            return child;
        }
        /* two children: replace with inorder successor */
        Book *s = bstMin(root->right);
        qClear(&root->waitlist);
        root->id = s->id;
        strcpy(root->title, s->title);
        strcpy(root->author, s->author);
        root->total = s->total;
        root->avail = s->avail;
        root->waitlist = s->waitlist;      /* move the queue over */
        qInit(&s->waitlist);               /* so freeing s keeps it intact */
        root->right = bstDelete(root->right, s->id);
    }
    return root;
}

void printBookRow(Book *b) {
    printf("  %-5d %-32s %-22s %d/%d   %d\n",
           b->id, b->title, b->author, b->avail, b->total, b->waitlist.size);
}

void printHeader(void) {
    printf("  %-5s %-32s %-22s %-6s %s\n", "ID", "Title", "Author", "Avail", "Waiting");
    printf("  ---------------------------------------------------------------------------\n");
}

void inorder(Book *root) {
    if (!root) return;
    inorder(root->left);
    printBookRow(root);
    inorder(root->right);
}

void collect(Book *root, Book **arr, int *n) {
    if (!root) return;
    collect(root->left, arr, n);
    arr[(*n)++] = root;
    collect(root->right, arr, n);
}

/* case-insensitive substring test */
int containsCI(const char *hay, const char *needle) {
    size_t hl = strlen(hay), nl = strlen(needle);
    if (nl == 0) return 1;
    for (size_t i = 0; i + nl <= hl; i++) {
        size_t j = 0;
        while (j < nl && tolower((unsigned char)hay[i + j]) == tolower((unsigned char)needle[j])) j++;
        if (j == nl) return 1;
    }
    return 0;
}

int keywordSearch(Book *root, const char *kw) {
    if (!root) return 0;
    int found = keywordSearch(root->left, kw);
    if (containsCI(root->title, kw) || containsCI(root->author, kw)) {
        printBookRow(root); found++;
    }
    return found + keywordSearch(root->right, kw);
}

/* ------------------------- MERGE SORT (by title) ------------------------- */
void merge(Book **a, Book **tmp, int l, int m, int r) {
    int i = l, j = m + 1, k = l;
    while (i <= m && j <= r) {
        if (strcasecmp(a[i]->title, a[j]->title) <= 0) tmp[k++] = a[i++];
        else tmp[k++] = a[j++];
    }
    while (i <= m) tmp[k++] = a[i++];
    while (j <= r) tmp[k++] = a[j++];
    for (k = l; k <= r; k++) a[k] = tmp[k];
}

void mergeSort(Book **a, Book **tmp, int l, int r) {
    if (l >= r) return;
    int m = (l + r) / 2;
    mergeSort(a, tmp, l, m);
    mergeSort(a, tmp, m + 1, r);
    merge(a, tmp, l, m, r);
}

/* ------------------------- LINKED LIST (issued books) ------------------------- */
typedef struct Loan {
    int bookId;
    struct Loan *next;
} Loan;

/* ------------------------- HASH TABLE (members) ------------------------- */
typedef struct Member {
    int id;
    char name[48];
    Loan *loans;
    int loanCount;
    struct Member *next;      /* chain */
} Member;

Member *table[HASH_SIZE];

int hashFn(int id) { return id % HASH_SIZE; }

Member *findMember(int id) {
    for (Member *m = table[hashFn(id)]; m; m = m->next)
        if (m->id == id) return m;
    return NULL;
}

int addMember(int id, const char *name) {
    if (findMember(id)) return 0;
    Member *m = malloc(sizeof(Member));
    m->id = id;
    strncpy(m->name, name, 47); m->name[47] = '\0';
    m->loans = NULL; m->loanCount = 0;
    int h = hashFn(id);
    m->next = table[h];       /* insert at head of chain */
    table[h] = m;
    return 1;
}

int hasLoan(Member *m, int bookId) {
    for (Loan *l = m->loans; l; l = l->next)
        if (l->bookId == bookId) return 1;
    return 0;
}

void addLoan(Member *m, int bookId) {
    Loan *l = malloc(sizeof(Loan));
    l->bookId = bookId; l->next = m->loans;
    m->loans = l; m->loanCount++;
}

int removeLoan(Member *m, int bookId) {
    Loan *prev = NULL, *cur = m->loans;
    while (cur && cur->bookId != bookId) { prev = cur; cur = cur->next; }
    if (!cur) return 0;
    if (prev) prev->next = cur->next; else m->loans = cur->next;
    free(cur); m->loanCount--;
    return 1;
}

int memberHoldsAny(Book *root, int bookId) {
    (void)root;
    for (int i = 0; i < HASH_SIZE; i++)
        for (Member *m = table[i]; m; m = m->next)
            if (hasLoan(m, bookId)) return 1;
    return 0;
}

/* ------------------------- STACK (transactions / undo) ------------------------- */
typedef enum { T_ISSUE, T_RETURN } TxnType;

typedef struct Txn {
    TxnType type;
    int memberId, bookId;
    struct Txn *next;
} Txn;

Txn *txnTop = NULL;

void pushTxn(TxnType t, int memberId, int bookId) {
    Txn *x = malloc(sizeof(Txn));
    x->type = t; x->memberId = memberId; x->bookId = bookId;
    x->next = txnTop; txnTop = x;
}

void popTxn(void) {
    if (!txnTop) return;
    Txn *x = txnTop; txnTop = x->next; free(x);
}

/* ------------------------- CORE OPERATIONS ------------------------- */
void issueBook(int mid, int bid) {
    Member *m = findMember(mid);
    Book *b = bstSearch(catalogue, bid);
    if (!m) { printf("  Member %d not found.\n", mid); return; }
    if (!b) { printf("  Book %d not found.\n", bid); return; }
    if (m->loanCount >= MAX_LOAN) { printf("  Limit of %d books reached.\n", MAX_LOAN); return; }
    if (hasLoan(m, bid)) { printf("  %s already has this book.\n", m->name); return; }
    if (b->avail == 0) {
        printf("  No copies available. Use 'Reserve' to join the waitlist.\n");
        return;
    }
    if (b->waitlist.size > 0) {
        if (b->waitlist.front->memberId != mid) {
            printf("  Copy is held for the next member in the waitlist (ID %d).\n",
                   b->waitlist.front->memberId);
            return;
        }
        dequeue(&b->waitlist);
    }
    b->avail--;
    addLoan(m, bid);
    pushTxn(T_ISSUE, mid, bid);
    printf("  Issued \"%s\" to %s.\n", b->title, m->name);
}

void returnBook(int mid, int bid) {
    Member *m = findMember(mid);
    Book *b = bstSearch(catalogue, bid);
    if (!m) { printf("  Member %d not found.\n", mid); return; }
    if (!b) { printf("  Book %d not found.\n", bid); return; }
    if (!removeLoan(m, bid)) { printf("  %s does not have this book.\n", m->name); return; }
    b->avail++;
    pushTxn(T_RETURN, mid, bid);
    printf("  \"%s\" returned by %s.\n", b->title, m->name);
    if (b->waitlist.size > 0) {
        Member *next = findMember(b->waitlist.front->memberId);
        printf("  Notify: %s (ID %d) is next in the waitlist.\n",
               next ? next->name : "?", b->waitlist.front->memberId);
    }
}

void reserveBook(int mid, int bid) {
    Member *m = findMember(mid);
    Book *b = bstSearch(catalogue, bid);
    if (!m) { printf("  Member %d not found.\n", mid); return; }
    if (!b) { printf("  Book %d not found.\n", bid); return; }
    if (hasLoan(m, bid)) { printf("  %s already has this book.\n", m->name); return; }
    if (inQueue(&b->waitlist, mid)) { printf("  Already in the waitlist.\n"); return; }
    if (b->avail > 0 && b->waitlist.size == 0) {
        printf("  Copies are available - just issue it directly.\n");
        return;
    }
    enqueue(&b->waitlist, mid);
    printf("  %s added to waitlist for \"%s\" (position %d).\n",
           m->name, b->title, b->waitlist.size);
}

void undoLast(void) {
    if (!txnTop) { printf("  Nothing to undo.\n"); return; }
    Member *m = findMember(txnTop->memberId);
    Book *b = bstSearch(catalogue, txnTop->bookId);
    if (!m || !b) { printf("  Cannot undo: record no longer exists.\n"); popTxn(); return; }
    if (txnTop->type == T_ISSUE) {
        removeLoan(m, b->id);
        b->avail++;
        printf("  Undid ISSUE of \"%s\" to %s.\n", b->title, m->name);
    } else {
        if (b->avail == 0 || m->loanCount >= MAX_LOAN) {
            printf("  Cannot undo this return right now (no copy free or member at limit).\n");
            return;
        }
        addLoan(m, b->id);
        b->avail--;
        printf("  Undid RETURN of \"%s\" by %s.\n", b->title, m->name);
    }
    popTxn();
}

void showHistory(void) {
    if (!txnTop) { printf("  No transactions yet.\n"); return; }
    printf("  Most recent first:\n");
    int i = 1;
    for (Txn *t = txnTop; t; t = t->next, i++) {
        Member *m = findMember(t->memberId);
        Book *b = bstSearch(catalogue, t->bookId);
        printf("  %2d. %-6s  %-14s  %s\n", i,
               t->type == T_ISSUE ? "ISSUE" : "RETURN",
               m ? m->name : "(unknown)", b ? b->title : "(removed book)");
    }
}

void showMember(int id) {
    Member *m = findMember(id);
    if (!m) { printf("  Member %d not found.\n", id); return; }
    printf("  Member %d: %s  (hash bucket %d)\n", m->id, m->name, hashFn(id));
    if (!m->loans) { printf("  No books issued.\n"); return; }
    printf("  Books issued:\n");
    for (Loan *l = m->loans; l; l = l->next) {
        Book *b = bstSearch(catalogue, l->bookId);
        printf("    - [%d] %s\n", l->bookId, b ? b->title : "(removed)");
    }
}

void showWaitlist(int bid) {
    Book *b = bstSearch(catalogue, bid);
    if (!b) { printf("  Book %d not found.\n", bid); return; }
    if (!b->waitlist.front) { printf("  Waitlist for \"%s\" is empty.\n", b->title); return; }
    printf("  Waitlist for \"%s\":\n", b->title);
    int pos = 1;
    for (QNode *n = b->waitlist.front; n; n = n->next, pos++) {
        Member *m = findMember(n->memberId);
        printf("    %d. %s (ID %d)\n", pos, m ? m->name : "?", n->memberId);
    }
}

void showSortedByTitle(void) {
    if (!catalogue) { printf("  Catalogue is empty.\n"); return; }
    Book **arr = malloc(sizeof(Book *) * bookCount);
    Book **tmp = malloc(sizeof(Book *) * bookCount);
    int n = 0;
    collect(catalogue, arr, &n);
    mergeSort(arr, tmp, 0, n - 1);
    printHeader();
    for (int i = 0; i < n; i++) printBookRow(arr[i]);
    free(arr); free(tmp);
}

/* ------------------------- INPUT HELPERS ------------------------- */
void readLine(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    if (!fgets(buf, size, stdin)) { buf[0] = '\0'; return; }
    buf[strcspn(buf, "\n")] = '\0';
}

int readInt(const char *prompt) {
    char buf[32];
    readLine(prompt, buf, sizeof buf);
    return atoi(buf);
}

void freeTree(Book *r) {
    if (!r) return;
    freeTree(r->left); freeTree(r->right);
    qClear(&r->waitlist); free(r);
}

void freeAll(void) {
    freeTree(catalogue);
    for (int i = 0; i < HASH_SIZE; i++) {
        Member *m = table[i];
        while (m) {
            Member *nx = m->next;
            while (m->loans) { Loan *l = m->loans; m->loans = l->next; free(l); }
            free(m); m = nx;
        }
    }
    while (txnTop) popTxn();
}

void seedData(void) {
    int ok;
    struct { int id; const char *t, *a; int c; } s[] = {
        {105, "Introduction to Algorithms",   "Cormen et al.",    2},
        {102, "The C Programming Language",   "Kernighan, Ritchie",1},
        {110, "Data Structures Using C",      "Reema Thareja",    3},
        {101, "Clean Code",                   "Robert Martin",    1},
        {108, "Discrete Mathematics",         "Kenneth Rosen",    2},
        {112, "Operating System Concepts",    "Silberschatz",     2},
        {104, "Database System Concepts",     "Korth",            1},
    };
    for (size_t i = 0; i < sizeof s / sizeof s[0]; i++) {
        catalogue = bstInsert(catalogue, newBook(s[i].id, s[i].t, s[i].a, s[i].c), &ok);
        bookCount++;
    }
    addMember(1, "Arjun");
    addMember(2, "Ram");
    addMember(12, "Ravi");
}

void menu(void) {
    printf("\n================ LIBRARY MANAGEMENT SYSTEM ================\n");
    printf("  BOOKS                         MEMBERS\n");
    printf("   1. Add book                   8. Register member\n");
    printf("   2. Remove book                9. View member / issued books\n");
    printf("   3. Search by ID  (BST)\n");
    printf("   4. Search by keyword          TRANSACTIONS\n");
    printf("   5. List all (by ID, inorder) 10. Issue book\n");
    printf("   6. List sorted by title      11. Return book\n");
    printf("      (merge sort)              12. Reserve book (queue)\n");
    printf("   7. View waitlist             13. Undo last transaction (stack)\n");
    printf("                                14. Transaction history\n");
    printf("   0. Exit\n");
    printf("===========================================================\n");
}

int main(void) {
    seedData();
    char a[64], b[64];
    int choice;
    do {
        menu();
        choice = readInt("Choice: ");
        switch (choice) {
        case 1: {
            int id = readInt("  Book ID: ");
            readLine("  Title: ", a, sizeof a);
            readLine("  Author: ", b, sizeof b);
            int copies = readInt("  Copies: ");
            if (id <= 0 || copies <= 0 || !a[0]) { printf("  Invalid input.\n"); break; }
            int ok = 0;
            Book *nb = newBook(id, a, b, copies);
            catalogue = bstInsert(catalogue, nb, &ok);
            if (ok) { bookCount++; printf("  Book added.\n"); }
            else { free(nb); printf("  A book with this ID already exists.\n"); }
            break;
        }
        case 2: {
            int id = readInt("  Book ID to remove: ");
            Book *bk = bstSearch(catalogue, id);
            if (!bk) { printf("  Book not found.\n"); break; }
            if (bk->avail != bk->total || memberHoldsAny(catalogue, id)) {
                printf("  Cannot remove: copies are currently issued.\n"); break;
            }
            catalogue = bstDelete(catalogue, id);
            bookCount--;
            printf("  Book removed.\n");
            break;
        }
        case 3: {
            Book *bk = bstSearch(catalogue, readInt("  Book ID: "));
            if (bk) { printHeader(); printBookRow(bk); } else printf("  Not found.\n");
            break;
        }
        case 4:
            readLine("  Keyword (title/author): ", a, sizeof a);
            printHeader();
            if (!keywordSearch(catalogue, a)) printf("  No matches.\n");
            break;
        case 5:
            if (!catalogue) printf("  Catalogue is empty.\n");
            else { printHeader(); inorder(catalogue); }
            break;
        case 6: showSortedByTitle(); break;
        case 7: showWaitlist(readInt("  Book ID: ")); break;
        case 8: {
            int id = readInt("  Member ID: ");
            readLine("  Name: ", a, sizeof a);
            if (id <= 0 || !a[0]) { printf("  Invalid input.\n"); break; }
            printf(addMember(id, a) ? "  Member registered.\n" : "  Member ID already exists.\n");
            break;
        }
        case 9: showMember(readInt("  Member ID: ")); break;
        case 10: { int m = readInt("  Member ID: "); issueBook(m, readInt("  Book ID: ")); break; }
        case 11: { int m = readInt("  Member ID: "); returnBook(m, readInt("  Book ID: ")); break; }
        case 12: { int m = readInt("  Member ID: "); reserveBook(m, readInt("  Book ID: ")); break; }
        case 13: undoLast(); break;
        case 14: showHistory(); break;
        case 0: printf("Goodbye!\n"); break;
        default: printf("  Invalid choice.\n");
        }
    } while (choice != 0);
    freeAll();
    return 0;
}
