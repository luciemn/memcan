# memscan: Forensic Keyword Scanner (C)

Petit scanner forensic en C: lit une image disque binaire (`disk.img`), charge une liste de mots-clés (`wordlist.txt`), puis cherche toutes les occurrences et affiche l’offset + le contexte autour du match.

## Fichiers

- `memscan.c`: programme principal
- `disk.img`: image binaire à analyser (dump)
- `wordlist.txt`: liste de mots-clés (1 par ligne, `#` = commentaire)

## Commandes MSYS2 (MINGW64)

```bash
cd /d/ESGI/3eme_annee/trimestre_deux/c/examen/memcan
gcc -Wall -Wextra -pedantic -std=c11 -g -o memscan memscan.c
./memscan usb.img wordlist.txt
```

Avec `Makefile`:

```bash
cd /d/ESGI/3eme_annee/trimestre_deux/c/examen/memcan
make
make run
```

Si `gcc`/`make` manquent:

```bash
pacman -S --needed mingw-w64-x86_64-gcc make
```

## Réponse aux questions

## Ex 1.2 — Read the image in chunks

- a) `unsigned char*` est adapte au binaire (0-255), sans probleme de signe. Avec `char`, certaines valeurs peuvent devenir negatives selon la plateforme.
- b) `fread` renvoie le nombre d'elements lus; en EOF: `0`. Si le dernier bloc est incomplet, on peut avoir une valeur non nulle mais inferieure a `CHUNK_SIZE`.
- c) En `"r"` sous Windows, conversion texte possible (`\r\n`), donc offsets faux. Pour une image disque, il faut donc `"rb"` pour lire les octets exacts.

## Ex 2.1 — Head insertion

Avant (head ->):
`"admin" -> "passwd" -> NULL`

Apres insertion en tete de `"secret"` :
`"secret" -> "admin" -> "passwd" -> NULL`

- Queue = O(N) car on parcourt jusqu'au dernier noeud.
- Tete: O(1), simple, mais ordre inverse.
- Queue: utile pour garder l'ordre d'insertion (ou avec pointeur `tail` pour rester en O(1)).

## Ex 2.3 — Pitfall `free(head); head = head->next;`

Apres `free(head)`, acces a `head->next` interdit (use-after-free).  
Il faut sauvegarder `next` avant `free` :

```c
next = head->next;
free(head);
head = next;
```

## Ex 2.5 — Pourquoi copier le contexte dans chaque `MatchNode` ?

Le buffer `buf` est reutilise puis libere.  
Si on stocke juste un pointeur, le contexte devient invalide.  
Donc chaque match doit avoir sa copie (`malloc` + `memcpy`) pour conserver une preuve stable du hit.

## Ex 3.1 — Pointer arithmetic warm-up

`data = { 'p', 'a', 's', 's', 0x00, 'A' }`

- (a) `*p` -> `p`
- (b) `*(p + 3)` -> `data[3]` -> `s`
- (c) `p[4]` -> `data[4]` -> `0` (affiche en entier)
- (d) `(data + 6) - p` -> `6`
- (e) apres `p++`, `*p` -> `data[1]` -> `a`

## Ex 3.2 — Byte scanner (questions)

- a) `end - ptr` = nb d'octets restants dans le chunk.
- b) `memcmp(...)`; retour `0` si match.
- c) `ptr - buf` = offset local, `file_pos` = offset du chunk dans le fichier, somme = offset absolu. C'est ce qui permet d'avoir des offsets fiables sur tout le dump.

## Ex 3.3 — Capturing context bytes

- a) `ctx_len + 1` pour garder un `'\0'` final (affichage propre).
- b) `ptr - before_len` pointe le debut du contexte gauche (ex: pos 10, ctx 4 -> debut a 6).
- c) Sans check, on peut lire avant `buf` (out-of-bounds), avec risque de crash ou d'erreur Valgrind.

## Ex 3.4 — Pointer vs array notation

- `*(ptr - 3)` <-> `ptr[-3]`
- `end[-1]` <-> `*(end - 1)`
- `ptr[0]` <-> `*ptr`

Si `ptr` est `unsigned char *`:

- le type de `ptr + 1` est `unsigned char *`
- l'adresse augmente de 1 octet.

## Ex 3.5 — Memory management audit

Tableau:

- Read buffer (`buf`) — alloue dans `scan_image()` — libere par `free(buf)` dans `scan_image()`
- Chaque `KeywordNode` — alloue dans `keyword_push()` — libere par `keyword_free()`
- Chaque `MatchNode` — alloue dans `match_push()` — libere par `match_free()`
- `node->context_before` — alloue dans `match_push()` — libere par `match_free()`
- `node->context_after` — alloue dans `match_push()` — libere par `match_free()`

### a. Memory leak = quoi ? Exemple

Memoire allouee mais jamais liberee.  
Ex: oublier `free(node->context_after)` dans `match_free()`.

### b. Use-after-free = quoi ? snippet

```c
KeywordNode *n = head;
while (n) {
    free(n);
    n = n->next;  // ERREUR: n est deja free
}
```

### c. Buffer overflow = quoi ? Ou dans memscan si on enleve les checks ?

Acces hors limites d'un buffer.  
Ici possible si on retire les verifications avant `memcpy`/`memcmp` (debut/fin de chunk).

## Ex 3.6 — Validation finale (attendu)

- Les 12 fragments doivent etre detectes avec les bons offsets (comparaison avec le script fourni).
- `valgrind --leak-check=full` doit indiquer 0 fuite.
- Compilation propre avec `-Wall -Wextra -pedantic` (0 warning).
