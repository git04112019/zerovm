/*
 * accounting of the user resources: local i/o, network i/o and cpu
 *
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

#ifndef ACCOUNTING_H_
#define ACCOUNTING_H_

#include "src/channels/channel.h"

/* update get statistics */
void CountGet(struct Connection *c, int size);

/* update put statistics */
void CountPut(struct Connection *c, int size);

/*
 * returns string with intermediate time and i/o statistics
 * WARNING: returned string should be deallocated with g_free
 */
char *FastAccounting();

/*
 * returns string with final time and i/o statistics
 * WARNING: returned string should be deallocated with g_free
 * and the function should be called only once
 */
char *FinalAccounting();

/* reset accounting internals */
void ResetAccounting();

#endif /* ACCOUNTING_H_ */
