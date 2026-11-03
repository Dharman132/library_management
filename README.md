# Library Management System (C)

A menu-driven library system for the Data Structures lab at Symbiosis Institute of Technology, Hyderabad. Every feature is backed by a different data structure.

| Data structure | Used for |
|---|---|
| Binary search tree | Book catalogue keyed by book ID (insert, search, delete, inorder) |
| Hash table (chaining) | Member lookup by member ID (`id % 11`) |
| Singly linked list | Books currently issued to each member |
| Queue | Waitlist per book, first reserved is first served |
| Stack | Transaction history and undo |
| Merge sort | Catalogue listed by title |
| Tree traversal | Keyword search in title and author |

## Files

| File | What it is |
|---|---|
| `library.c` | Complete source code |
| `Makefile` | Build, run and demo targets |
| `demo_input.txt`, `demo_output.txt` | Scripted demo input and its recorded output |
| `Library_Management_System_Report.pdf` / `.docx` | Project report |
| `Library_Management_System_Presentation.pptx` | Presentation |

## Build and run

```bash
make        # builds ./library
make run    # start the program
make demo   # replay demo_input.txt
```

Or compile by hand: `gcc -Wall -Wextra -o library library.c`. Works on Linux, macOS, and Windows with MinGW or WSL. The program starts with 7 books and 3 members (Arjun ID 1, Ram ID 2, Ravi ID 12).

## Rules enforced

- A member can hold at most 3 books.
- A member cannot issue the same book twice.
- A book with no free copy must be reserved, not issued.
- A returned copy is held for the first member in the waitlist.
- A book cannot be removed while a copy is issued.
- Duplicate book and member IDs are rejected.

## Try this

1. Issue book 102 to member 1 (option 10).
2. Reserve it for members 2 and 12 (option 12).
3. Return it (option 11); member 2 is named as next.
4. Try to issue it to member 12; it is refused.
5. Undo the return (option 13).

## Limitations

Data is kept in memory only. The tree is not self-balancing. Future work: save and load from a file, due dates and fines, an AVL tree.

## Authors

Dharman Khambhalia (PRN 25070721014) and Anshuman Yadav (PRN 25070721007)
