#include <linux/sys.h>
#include <linux/linkage.h>

.text

ENTRY(asm_probe)
  push %rbp
  mov %rsp, %rbp
  // YOLO
  cli
  // Set list
  mov %rax, %r9
  // Set size
  mov %rbx, %r10
  // Candidate
  mov %rcx, %r11
  // Load candidate cache line
  mov (%r11), %rax
//normal_probe:
//  // for (i = 0; i < ss; i++)
//  // i = 0
//  xor %r12, %r12
//loop:
//  cmp %r12, %r10
//  jz endloop
//  // %rax = set[i]
//  // lea (%r9), %r15
//  mov (%r9), %r9
//  // i++
//  inc %r12
//  jmp loop
//endloop:
yolo_probe:
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
  mov (%r9), %r9
end_yolo_probe:
  mfence
  lfence
  rdtsc
  mov %eax, %edi
  // Access candidate cache line
  mov (%r11), %rax
  mfence
  lfence
  rdtsc
  sub %edi, %eax
  mov %r9, %rcx
  lea (%rip), %rdx
  // NO YOLO
  cli
  leave
  retq
