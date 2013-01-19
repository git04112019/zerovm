/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
/*
 * Copyright (c) 2012, LiteStack, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include "src/platform/portability.h"
#include "src/main/nacl_globals.h"
#include "src/syscalls/nacl_syscall_handlers.h"
#include "src/platform/sel_memory.h"
#include "src/syscalls/nacl_syscalls.h"
#include "src/syscalls/trap.h"
#include "src/main/manifest_setup.h"

struct NaClSyscallTableEntry nacl_syscall[NACL_MAX_SYSCALLS] = {{0}};

static int32_t NotImplementedDecoder(struct NaClApp *nap)
{
  UNREFERENCED_PARAMETER(nap);
  return -ENOSYS;
}

static void NaClAddSyscall(int num, int32_t(*fn)(struct NaClApp *))
{
  ZLOGFAIL(nacl_syscall[num].handler != &NotImplementedDecoder,
      EFAULT, "Duplicate syscall number %d", num);
  nacl_syscall[num].handler = fn;
}

/*
 * syscalls main routines.
 * note that we only need 1 syscall trap() other syscalls are only used
 * for nexe prolog in a future all syscalls except trap can be removed.
 */
int32_t NaClSysSysbrk(struct NaClApp *nap, uintptr_t new_break)
{
  if(new_break >= nap->data_end && new_break < nap->heap_end)
    nap->break_addr = new_break;
  return (int32_t)nap->break_addr;
}

/* d'b: duplicate of trap() exit. remove it when trap() replace syscalls */
int32_t NaClSysExit(struct NaClApp *nap, int status)
{
  ZLOGS(LOG_DEBUG, "Exit syscall handler: %d", status);
  nap->system_manifest->user_ret_code = status;
  longjmp(user_exit, status);

  /* NOTREACHED */
  return -EINVAL;
}

/* tls create */
int32_t NaClSysTls_Init(struct NaClApp *nap, void *thread_ptr)
{
  uintptr_t sys_tls;

  ZLOGS(LOG_DEBUG, "tls init with 0x%08lx", (uintptr_t)thread_ptr);

  /*
   * Verify that the address in the app's range and translated from
   * nacl module address to service runtime address - a nop on ARM
   */
  sys_tls = NaClUserToSysAddrRange(nap, (uintptr_t) thread_ptr, 4);
  ZLOGS(LOG_INSANE, "thread_ptr 0x%p, sys_tls 0x%lx", thread_ptr, sys_tls);

  if(kNaClBadAddress == sys_tls) return -EFAULT;

  nap->sys_tls = sys_tls;
  return 0;
}

/* tls get */
int32_t NaClSysTls_Get(struct NaClApp *nap)
{
  uint32_t user_tls;

  /* too frequently used, and syscall-number level logging suffices */
  user_tls = (int32_t) NaClSysToUser(nap, nap->sys_tls);
  return user_tls;
}

/*
 * decoders. get call parameters from the user stack and call the syscall
 */

/* this function was automagically generated */
static int32_t NaClSysSysbrkDecoder(struct NaClApp *nap)
{
  struct NaClSysSysbrkArgs
  {
    uint32_t new_break;
  } p = *(struct NaClSysSysbrkArgs *) nap->syscall_args;

  return NaClSysSysbrk(nap, (uintptr_t) p.new_break);
}

/* this function was automagically generated */
static int32_t NaClSysExitDecoder(struct NaClApp *nap)
{
  struct NaClSysExitArgs
  {
    uint32_t status;
  } p = *(struct NaClSysExitArgs *) nap->syscall_args;

  return NaClSysExit(nap, (int) p.status);
}

/* this function was automagically generated */
static int32_t NaClSysTls_InitDecoder(struct NaClApp *nap)
{
  struct NaClSysTls_InitArgs
  {
    uint32_t thread_ptr;
  } p = *(struct NaClSysTls_InitArgs *) nap->syscall_args;

  return NaClSysTls_Init(nap, (void *) (uintptr_t) p.thread_ptr);
}

/* this function was automagically generated */
static int32_t NaClSysTls_GetDecoder(struct NaClApp *nap)
{
  return NaClSysTls_Get(nap);
}

/*
 * only needed by nexe prologue. if user need real functionality
 * zrt library can be used (in the future)
 */
static int32_t NaClSysMutex_CreateDecoder(struct NaClApp *nap)
{
  return 1;
}

/* d'b: trap(). see documentation for the details */
static int32_t TrapDecoder(struct NaClApp *nap)
{
  struct TrapDecoderArgs
  {
    uint32_t args;
  } p = *(struct TrapDecoderArgs*) nap->syscall_args;

  return TrapHandler(nap, p.args);
}

/*
 * d'b: see documentation about "one ring" tonneling syscall
 * note: all syscalls except trap() only need for nexe prolog
 */
void NaClSyscallTableInit()
{
  int i;

  for(i = 0; i < NACL_MAX_SYSCALLS; ++i)
    nacl_syscall[i].handler = &NotImplementedDecoder;

  NaClAddSyscall(Trap, &TrapDecoder); /* 0. added onering call */
  NaClAddSyscall(NACL_sys_sysbrk, &NaClSysSysbrkDecoder); /* 20 */
  NaClAddSyscall(NACL_sys_exit, &NaClSysExitDecoder); /* 30 */
  NaClAddSyscall(NACL_sys_tls_init, &NaClSysTls_InitDecoder); /* 82 */
  NaClAddSyscall(NACL_sys_tls_get, &NaClSysTls_GetDecoder); /* 84 */
  NaClAddSyscall(NACL_sys_mutex_create, &NaClSysMutex_CreateDecoder); /* 70 */
}