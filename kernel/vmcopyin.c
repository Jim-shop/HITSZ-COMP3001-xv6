#include "param.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "proc.h"

//
// This file contains copyin_new() and copyinstr_new(), the
// replacements for copyin and coyinstr in vm.c.
//

static struct stats {
  int ncopyin;
  int ncopyinstr;
} stats;

int statscopyin(char *buf, int sz) {
  int n;
  n = snprintf(buf, sz, "copyin: %d\n", stats.ncopyin);
  n += snprintf(buf + n, sz, "copyinstr: %d\n", stats.ncopyinstr);
  return n;
}

// Copy from user to kernel.
// Copy len bytes to dst from virtual address srcva in a given page table.
// Return 0 on success, -1 on error.
int copyin_new(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len) {
  struct proc *p = myproc();

  if (srcva >= p->sz || srcva + len >= p->sz || srcva + len < srcva) return -1;
  w_sstatus(r_sstatus() | SSTATUS_SUM);
  memmove((void *)dst, (void *)srcva, len);
  w_sstatus(r_sstatus() & ~SSTATUS_SUM);
  stats.ncopyin++;  // XXX lock
  return 0;
}

// Copy a null-terminated string from user to kernel.
// Copy bytes to dst from virtual address srcva in a given page table,
// until a '\0', or max.
// Return 0 on success, -1 on error.
int copyinstr_new(pagetable_t pagetable, char *dst, uint64 srcva, uint64 max) {
  struct proc *p = myproc();
  char *s = (char *)srcva;

  stats.ncopyinstr++;  // XXX lock
  w_sstatus(r_sstatus() | SSTATUS_SUM);
  for (int i = 0; i < max && srcva + i < p->sz; i++) {
    dst[i] = s[i];
    if (s[i] == '\0') {
      w_sstatus(r_sstatus() & ~SSTATUS_SUM);
      return 0;
    }
  }
  w_sstatus(r_sstatus() & ~SSTATUS_SUM);
  return -1;
}

int sync_pagetable(pagetable_t kpgtbl, pagetable_t upgtbl) {
  pagetable_t usubpgtbl = (pagetable_t)PTE2PA(upgtbl[0]);
  pagetable_t ksubpgtbl = (pagetable_t)PTE2PA(kpgtbl[0]);
  memmove(ksubpgtbl, usubpgtbl, 96 * sizeof(pte_t));
  return 0;
}
