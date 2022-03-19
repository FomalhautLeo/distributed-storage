# 引言
       
       该项目是2021年下旬我们参加腾讯的引力计划时布置的任务，实现了一个基于LRU机制的分布式缓存系统。
       
       感谢所有组员的支持，感谢耿哥和小杜陪我度过了那些不停debug的日日夜夜。由于我们都是非科班的同学，水平十分有限，加之当时时间紧迫，很多功能实现得并不是很完善。若能给大家提供一点点参考，荣幸之至~
       

# 1. 分布式缓存系统概述

​       受磁盘 I/O 性能的影响，数据库的检索速度十分有限。而随着服务器的扩大和客户端数据请求的增加，数据库的压力陡然增加，甚至出现无法正常响应的可能。该问题可以通过引入缓存来解决：利用时间局部性原理，可以对访问频率较高和近期访问的数据请求缓存至内存中。当请求再次发起时，就可以先从缓存中查询结果，从而在一定程度上减少对数据库（磁盘）的直接访问，以达到提高请求处理速度和减小服务器的压力的目的。但单点缓存的稳定性不足，难以满足大流量、高并发的服务环境。此外，单点缓存的可扩展性较弱，难以进行容量扩展操作。因此可以使用布式缓存系统，以提高系统的稳定性可扩展性，进而提高系统的可用性。
![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173405_dea104c6_10062397.png "屏幕截图.png")

|        | 无缓存                                         | 单点缓存                     | 分布式缓存                                 |
| ------ | ---------------------------------------------- | ---------------------------- | ------------------------------------------ |
| 稳定性 | 较差，数据库压力过大会使性能下降，导致请求超时 | 一般，有单点故障问题         | 较好，容灾机制保证了单点故障不影响整体使用 |
| 扩展性 | 较差                                           | 一般，受制于单服务器资源限制 | 较好，支持水平扩展                         |



# 2. 设计方案详述

​       本章将介绍该分布式缓存系统（**后简称为“系统”**）的各部分的详细设计方案及实现细节。



## 2.1 总体设计方案

### 2.1.1 系统结构

​       在默认工作环境下，系统结构如下：

- 2个 Client：负责读取数据、存储数据的节点各一个；
- 1个 Master：负责维护各 CacheServer 的状态、分发哈希槽以及其他工作的调度；
- 6个 CacheServer：分为3组，每组主节点、备节点各1个。主节点负责处理来自 Client 的数据访存请求，备节点负责存储备份数据以及参与容灾工作。支持通过扩容和缩容操作根据实际需求来改变 CacheServer 的数量。

### 2.1.2 系统功能

​       系统支持以下功能：

1. 通过哈希槽机制，保证数据大致均匀分布在各 CacheServer 中；
2. 通过 LRU 淘汰算法，保证内存使用的稳定，并存留最近访问的数据；
3. 支持数据备份以及 CacheServer 宕机重启后的数据恢复，进而保证了对于 CacheServer 的容灾功能，即某一 CacheServer 的崩溃至恢复正常工作状态期间，不影响 Client 的使用体验；
4. 支持哈希槽备份，进而保证了对于 Master 的容灾功能，即 Master 宕机，不影响 Client 的使用体验；
5. 支持对于 CacheServer 的扩容与缩容，即存储节点数量的增减，以满足更改系统数据容量的要求；
6. 系统的各节点均使用了日志系统，对输出标记时间戳，进行分级、标记不同的颜色，并将所有事件记录至文件中以便检查和回溯问题。

### 2.1.3 工作过程

​       默认工作环境下，Master 上线后开始监听外部连接。随后各组 CacheServer 主、备节点依次上线，Master 收到各 CacheServer 发送的心跳包，并根据各节点发送的编号来确定并告知其属于主节点/备节点。最后向所有 CacheServer 广播当前哈希槽。

​       Client 上线后，依次与 Master 和各 CacheServer 建立连接，并接收 Master 发送的哈希槽，保存在本地。

​       连接建立后，Client 会默认使用保存在本地的哈希槽。首先对于某个 Key 值，由哈希槽计算出该 Key 值应当归属的  CacheServer 编号（**后简称为" CacheNum "**），并将 Key 值发送到对应 CacheServer（若为存请求，则还需发送对应的 Value 值）。而后，某 CacheServer 收到数据的访存请求，并在本地的数据中查找并更新 LRU ：对于读请求，若找到对应的 Key 值，则返回其对应的 Value 值，否则返回“NULL”字符串；对于存请求，若该 Key 值已存储了数据，则更新其 Value 值，否则将该 Key-Value 数据对加入数据集，后通知 Client 存储成功。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173507_d9401121_10062397.png "屏幕截图.png")

​       对于系统容量变更需求的应对措施，详见2.4节。

​       对于某一服务端失效后的应对措施，详见2.5节。




## 2.2 数据均衡负载与  LRU

​       本节将讨论系统工作时与数据处理有关的两种机制：哈希槽算法与 LRU 算法。

### 2.2.1 哈希槽算法

​       单个 CacheServer 并发量和存储量有限，通过多台机器提高存储量时，则需要在 Master 上实现一种负载均衡策略，以将写入和读出的数据负载平均分配到不同的 CacheServer 上。在本系统中我们选择了哈希槽策略来实现负载均衡。


#### 2.2.1.1 哈希槽机制实现方法

​       哈希槽策略首先设置了槽的个数（系统中设置为16384），然后划分槽的分布，即划分每一个槽所属的 CacheServer。当某一 Key 值到来时，先对 Key 求CRC16 校验码，CRC 运算时，首先将一个16位的寄存器预置为全1，然后连续把数据帧中的每个字节中的8位与该寄存器的当前值进行运算，仅仅每个字节的8个数据位与生成 CRC，起始位和终止位以及可能使用的奇偶位都不影响 CRC。在生成 CRC 时，每个字节的8位与寄存器中的内容进行异或，然后将结果向低位移位，高位则用“0”补充，最低位（LSB）移出并检测，如果是1，该寄存器就与一个预设的固定值（0A001H）进行一次异或运算，如果最低位为0，不作任何处理。上述处理重复进行，直到执行完了8次移位操作，当最后一位（第8位）移完以后，下一个8位字节与寄存器的当前值进行异或运算，同样进行上述的另一个8次移位异或操作，当数据帧中的所有字节都作了处理，生成的最终值就是 CRC 校验码。最后将其对16384取模，如此该 Key 值就会落在某个槽上，进而可知其对应的 CacheNum。

​       Master 在初始化时设置一个空的哈希槽。当 CacheServer1 上线并发送心跳给 Master 时，哈希槽会将全部的槽分配给 CacheServer1。后 CacheServer2 上线并发送心跳给 Master，哈希槽将 CacheServer1 一半的槽分配给 CacheServer2。此后 CacheServer3上线并发送心跳时，哈希槽将 CacheServer1 和 CacheServer2 各1/3的槽分配给 CacheServer3，反之如果某一个 CacheServer 下线时，将其占有的槽平均分配给其他 CacheServer。以此类推，可以保证多个服务器上线后，每个服务器被分配到的槽的差至多为1，且在 CacheServer 数量变化时，不同的 CacheServer 的哈希槽的数量变化的差至多为1。
![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173518_12c2d603_10062397.png "屏幕截图.png")


#### 2.2.1.2 均衡负载验证

​       哈希槽具有良好的可扩展性和单调性，且在统计意义上没有数据倾斜的情况。下图展示了3个 CacheServer 在不同总数据量下数据分布差值的情况。横坐标为数据总量取2的对数，纵坐标为不同服务器数据量的极差占数据总量的百分比$E$。例如：某时刻A、B、C三个 CacheServer 的存储数据量分别为$p$、$q$、$r$，且有$p\leqslant q\leqslant r$，则：
$$
E=\frac{r-p}{p+q+r}\times100\%
$$

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173528_49470dfd_10062397.png "屏幕截图.png")

​       由上图可以看出，当数据量达到2^8^ (256) 时，$E<2\%$，数据分布已经基本均衡。



### 2.2.2  LRU 淘汰机制

​        CacheServer 通过  LRU （Least Recently Used 最近最少使用） 算法来控制单个 CacheServer 的数据总量，保证内存使用的稳定，并淘汰最近最不常被使用的数据。

​       CacheServer 的 LRU 机制通过一个哈希表和一个链表结合实现。其中链表记录 Key - Value 数据对，哈希表记录着某一 Key 值到其对应的 Key - Value 数据对在链表中迭代器的映射关系。对于某次访存请求，存在以下几种情况：

1. 读取一个未存储过或已淘汰 Key 对应的 Value 值：在哈希表中查找失败，直接返回“NULL”字符串；
![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173539_ae692716_10062397.png "屏幕截图.png")

   

2. 读取一个链表中存在的 Key 对应的 Value 值：在哈希表中查找 Key 值对应在链表中的位置（迭代器），在该位置取出 Value 值，后将该节点移到链表头部，并更新哈希表中该 Key 对应的迭代器信息，最后返回 Value 值；

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173546_2250e7ef_10062397.png "屏幕截图.png")

   

3. 更新一个已存在的 Key 值对应的 Value 值：与读取该 Key 值对应的 Value 值过程相似，只是需要更新链表中该 Key 值对应的 Value 值；

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173554_8f1a0cff_10062397.png "屏幕截图.png")

   

4. 存储一个未存储过或已被淘汰的 Key - Value 数据对：将该数据对插入链表头部，并将 Key 值及其对应的迭代器记录到哈希表中；若此时容量已满，则删除链表中最后一个节点，及其在哈希表中对应的信息。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173601_60236c49_10062397.png "屏幕截图.png")

​       上述算法无论是 Key 值对应链表迭代器的查询，还是数据的更新以及链表节点的移动或增删，均保证了数据存储和读取的时间复杂度为 $O(1)$。



## 2.3  Client 设计

​       系统中存在2个 Client 节点，一个负责向 CacheServer 中写入数据，另一个负责从 CacheServer 中读取数据。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173611_1677f66c_10062397.png "屏幕截图.png")


### 2.3.1 哈希槽本地缓存

​       Master 生成的哈希槽将被作为 Client 的本地缓存，每次发起数据访存请求时，对于某一个 Key 值，Client 可以直接根据本地的哈希槽计算出对应的 CacheNum，而不必再访问 Master 来获取数据分布，以达到减小 Master 服务器压力的目的。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173621_61b8aa33_10062397.png "屏幕截图.png")

#### 2.3.1.1 本地缓存实现方法

​       Client 上线后，首先向 Master 发起连接请求。连接成功后，Master 发回一个当前使用的哈希槽（即数据分布信息），Client 将该哈希槽保存在本地，在此后的数据访存请求中，Client 会一直使用该哈希槽来计算某一 Key 值对应的 CacheNum。若系统发生扩缩容等操作，需要变更哈希槽，Client 会接收从 Master 发送的新哈希槽并暂存。直至操作结束，Client 收到 Master 通知本次操作成功，则将暂存的新哈希槽更新为当前哈希槽，删除原有的旧哈希槽。

#### 2.3.1.2 本地缓存优势

​       Client 端只需要在上线时访问一次 Master 获取哈希槽即可，之后对哈希槽的更新仅会在 CacheServer 群组扩容/缩容情况下发生，这将大大降低 Client 对于 Master 的访问次数。此外，Client 设计了“有效节点切换”机制（见2.3.4），故某 CacheServer 发生宕机时群组数量保持不变，因此对于 CacheServer 的容灾处理不涉及到本地哈希槽缓存的更新。

### 2.3.2 数据随机生成

​       测试数据由TestDataGeneration类的GetNum()方法生成，其中维护了一个随机数容器，包含了用于生成随机 Key 和 Value 的共70个字符（0-9、A-Z、a-z以及8个其他符号）。Client 1进行写请求操作，即尝试向 CacheServer 中发送数据存储请求，成功后则向`Keylist.txt`文件中写入一个 Key，并在其中设置了每10个 Key 值中掺杂1个假 Key 值（没有存储 Value 值）。Client 2通过读取 Key list.txt中的 Key，向 CacheServer 发起对应的数据读取请求。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173633_7dfa1d39_10062397.png "屏幕截图.png")

### 2.3.3 数据访存

​        Client 端内部维护了一个哈希表，其中记录了从 CacheNum 到对应 CacheServer 套接字的映射关系。另外有一个哈希表，记录了从 CacheServer 的组编号到该组内“有效” CacheNum 的映射关系。此处“有效”一词的含义是： Client 在一次数据访存过程中，会将请求发送给某一组 CacheServer 中的一个节点，该节点即被认为是与 Client 进行通信的“有效节点”（绝大多数情形下，可以认为该节点就是其组内的主节点）。

​       对于一次某 Key 值的访存请求，首先通过本地的哈希槽计算出对应的 CacheServer 组，并将该请求发送给该组的有效节点，后等待其返回读请求对应的 Value 值或存请求的“存储成功”消息。若要请求读一个此前未存储过的或已经被 LRU 机制淘汰的 Key - Value 数据，CacheServer 端会返回“NULL”字符串。CacheServer 使用epoll处理请求，因此可以支持多个 Client 同时进行读写。

​       对于 Client 的数据访存在扩缩容时的解决方案，详见2.4节。

### 2.3.4  CacheServer 失效情况

​       若在数据访存请求中，某一 CacheServer 失效，会导致 Client 发送访存请求失败，或无法收到来自 CacheServer 的 Value 值或“存储成功”消息。此时，Client 会在记录有效节点的哈希表中，将该组的有效 CacheNum 切换为组内的另一个 CacheNum。并对新的有效节点重新发送相同的数据访存请求。由于在 CacheServer 的实现中，保证了主备节点的数据强一致性（数据备份成功后才会将对应消息发回 Client），该过程不会对 Client 的正常访存请求产生影响。

​       另外存在一种可能的情况，即一组 CacheServer 中的备节点宕机，但此时 Client 端不会收到任何通知，宕机节点重启后其与 Client 的连接依然处于断开状态。若此后一段时间，主节点宕机，Client 会触发有效节点切换操作。而此时若将前面宕机后重启的备节点作为有效节点，此时的连接仍是断开的，无法正常发送请求。为解决此问题，Client 在切换有效节点后，会检查其与该 CacheServer 的连接状态，若连接已经断开则重新发起连接。此后就可以进行正常的数据访存操作。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173658_b3ac8d44_10062397.png "屏幕截图.png")



## 2.4 扩容与缩容

### 2.4.1 Cache-Server存活监控

​       每个 CacheServer 上线后(无论主/备节点)，每秒会向 Master 发送一次心跳包，表明自己的存活状态，Master 收到 CacheServer 的心跳包后，会把对应 CacheServer 的计数器count置为0。此外，Master 内部维护了一个哈希表，长度与 CacheServer 的数量相同，哈希表的 Key 为 CacheServer 的编号 CacheNum，用于检索到记录每个 CacheServer 信息的 CacheInfo 实例对象。Master 每隔400ms会遍历检查哈希表中每个 CacheServer 的计数器，将计数器数值加一，同时检测是否有 CacheServer 的计数器 count 等于5。若有，说明此 CacheServer 超过2s没有发送心跳包，此时认定此 CacheServer 宕机，将其从哈希表中删除。若该节点为主节点，则通知此其组内的备节点升级为主节点。

### 2.4.2 Cache-Server群组扩容

​       CacheServer 群组扩容分为三个阶段：“扩容开始前”，“扩容进行中”和“扩容完成后”。

​       扩容开始前：每个 CacheServer 上线时，都会给 Master 发送自己的编号（CacheNum），为了避免系统启动时频繁的进行扩容操作，一开始会假定有三个 CacheServer 组，并用两个哈希表分别存储 CacheNum 首位为1和 CacheNum 首位为2的 CacheServer，然后进行初始化。需要扩容时，为系统加入一组 CacheServer，这组 CacheServer 上线后，Master 检测出这组 CacheServer 的编号超过了哈希表的范围，于是开始启动扩容操作。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173724_7f95b379_10062397.png "屏幕截图.png")

​       扩容进行中：当 CacheServer 扩容时，Master 一方面会保存旧的哈希槽，另一方面会重新分配一个新的哈希槽，再将最新的哈希槽、旧 CacheServer 组的数量、新 CacheServer 组的编号发送给新的 CacheServer 组，将最新的哈希槽、旧 CacheServer 组的数量、新 CacheServer 组的编号、新 CacheServer 组为接收旧 CacheServer 组数据设置的监听端口号发送至所有旧 CacheServer 组，同时将新的哈希槽广播至每个 Client 端。然后等待所有客户端和 CacheServer 回复成功接收哈希槽的消息，收到反馈消息后，广播告知所有 CacheServer 开始进行数据迁移，并等待新 CacheServer 组接收数据完成后反馈的确认消息。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173735_1f71d0fb_10062397.png "屏幕截图.png")

​       扩容完成后： Master 收到数据迁移完成的确认消息后，更新本地哈希槽，并向 Client 端和 CacheServer 发送扩容成功通知。Client 端和 CacheServer 收到扩容成功的通知后，更新本地哈希槽。

### 2.4.3  CacheServer 群组缩容

​       缩容是通过一个键盘输入信号的监听来模拟人工进行的缩容操作，按下ESC键后，再键盘输入要删除的 CacheServer 的编号，启动缩容操作。

​       缩容操作与扩容流程大致相同，Master 保存原始哈希槽后，分配新的哈希槽，并将新的哈希槽广播至 Client 端和 CacheServer 端，Master 收到 Client 端和 CacheServer 的确认反馈后，再告知所有 CacheServer 进行数据迁移，并等待 CacheServer 数据迁移的成功反馈。Master 收到数据迁移成功反馈后，更新本地哈希槽，广播缩容成功消息，并告知被删除的 CacheServer 自动结束进程。Client 端和 CacheServer 收到缩容成功消息后，更新本地哈希槽。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173750_1beac051_10062397.png "屏幕截图.png")

### 2.4.4. CacheServer 扩容时的数据迁移

​       在扩容之前，所有的 CacheServer 会收到 Master 发送的新哈希槽，作为将来判断自身存储的 Key-Value 数据对值是否变更所有权为新 CacheServer 的依据。由于在我们的系统中各个 CacheServer 的负载是均衡的，且当新 CacheServer 上线时，每一个原有的 CacheServer 发送给新 CacheServer 的数据的总量大致相当，因此使用多线程的方式进行数据的迁移。

​       在所有 CacheServer 收到数据迁移的指令后，原有的 CacheServer 会同时通过遍历全部 Key-Value 数据对并计算每一个 Key 是否还属于自身，暂存不再属于自身的数据，最后统一向新 CacheServer 发送数据，在原有的 CacheServer 接收到 Master 发送的扩容成功的通知后，删除这些暂存的数据；新 CacheServer 会根据原有的 CacheServer 的数量创建线程，在多个子线程中使用不同的socket分别接收不同的原有的 CacheServer 发送来的数据并暂存在临时的 LRU 缓存中。最后当所有子线程结束后，主线程会合并所有的临时 LRU 缓存，作为自身的 LRU 缓存。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173759_0c1eba18_10062397.png "屏幕截图.png")

### 2.4.5  CacheServer 缩容时的数据迁移

​       在缩容之前，所有的 CacheServer 会收到 Master 发送的新哈希槽。将被删除的 CacheServer 将使用新哈希槽来决定自身所有的 Key 属于哪个原有的 CacheServer。在这一过程中，需要对将要被删除的 CacheServer 中的全部 Key-Value 数据对进行一次遍历，分别归档属于不同依然保留的 CacheServer 的 Key-Value 数据对并分别发送。

​       对依然保留的 CacheServer，接收将被删除的 CacheServer 的 Key-Value 数据对，并与本地 LRU 缓存进行合并。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173819_8ae494d2_10062397.png "屏幕截图.png")

### 2.4.6 CacheServer 扩缩容时的 Client 请求应对

​       在扩缩容时，Client 依然会对 CacheServer 的 LRU 缓存进行读写，然而在扩缩容时 CacheServer 会对自身的 LRU 缓存进行写操作，这样就可能造成数据竞争。除此之外，如果负责读取数据的 Client 的读取速度与另一个 Client 的写入数据相当，就可能造成写入的 Client 把某些 Key-Value 数据对刚刚写入后收到扩缩容通知，此时读取数据的 Client 按照新哈希槽的计算结果访问 CacheServer 就会得到错误的结果。

​       为避免这种情况，在 CacheServer 的扩缩容方法中加入了标志当前 CacheServer 是否在进行扩缩容引起的读写 LRU 缓存的操作的变量，当 Client 试图访问该变量为真的 CacheServer 时会被拒绝。此时的 CacheServer 处于忙状态。

​       对于扩容操作，新 CacheServer 的读写 LRU 缓存的时间一般来说要长于原有的 CacheServer，而由于负载均衡的原因，只有小部分的读写请求会访问新 CacheServer，因此在Clinet中设置了一个临时的缓存，当某一读写请求访问的 CacheServer 正忙时，会将其存入缓存中之后再处理。当负责写入的 Client 收到系统扩容升级成功的通知后，会开始处理积攒的临时缓存，处理完成后会通知负责读取的 Client，之后后者也开始处理积攒的临时缓存以保证读取数据的有效性。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173829_33685ed4_10062397.png "屏幕截图.png")

​       对于缩容操作，由于在本系统中各个 CacheServer 具有全同性，因此各个依然保留的服务器处于忙状态的时间范围大致相当，设置临时缓存没有意义，故直接阻塞所有写入和读取操作。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173837_51492195_10062397.png "屏幕截图.png")

## 2.5 服务器容灾

### 2.5.1 CacheServer 的容灾

#### 2.5.1.1 数据备份

​       CacheServer 在处理来自 Client 的访存请求时，保持了备份数据的强一致性：主节点在接收到 Client 的请求后，处理该请求，并将此请求转发给备节点。收到备节点的应答后，才回发给 Client 本次的处理结果。

#### 2.5.1.2 节点宕机处理

​       若主节点宕机，其不再向 Master 发送心跳包。Master 发现该情况后，会告知该 CacheServer 内的备节点升级为主节点。备节点升级后，等待宕机节点重新上线，并处理来自 Client 的数据访存请求。宕机节点重启后，自动降为备节点，此时主节点（之前进行升级的备节点）监测到备节点重新上线，会将本地存储的数据恢复到备节点。

​       若备节点宕机，Master 会监测到其停止心跳，进行记录，但不作实质应对。此时主节点发现发送备份数据失败，便不再进行数据备份，并等待备节点重新上线。备节点上线后，开始数据恢复操作。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173847_40d57349_10062397.png "屏幕截图.png")

#### 2.5.1.3 数据恢复

​       数据恢复就是将本地的数据以写请求的方式倒序发送给新上线节点的过程（倒序保证了最新的数据最后发送），在实现上使用了子进程进行，利用`fork()`系统调用的“写时复制”机制，若在数据恢复过程中无客户端新请求的到达，则子进程仅需使用父子进程共享的空间，而无需额外复制数据进行恢复转发，不会造成空间浪费。

​       若在数据恢复过程中，客户端发来了新的数据访存请求，主节点同样会将该请求转发给备节点，但备节点不会马上处理该请求，而是将其存储到一个队列中。直到数据恢复完成后，备节点才会将存在队列中的新请求依次取出并处理。队列为空后，整个数据恢复过程结束，主备节点均回到正常工作状态。

![输入图片说明](https://images.gitee.com/uploads/images/2021/1221/173856_4a55584f_10062397.png "屏幕截图.png")

### 2.5.2 Master 的容灾

​       Master 在正常工作时，会将哈希槽广播到所有 Client 端和 CacheServer 端。因此 Master 的宕机只会影响到检测 CacheServer 的心跳和异常情况的处理。即在正常服务环境下，Master 宕机不会影响到 Client 正常进行数据访存。



# 3. 存在不足



## 3.1  Master 状态恢复




### 3.1.1 问题描述

​       系统未支持 Master 在宕机重启后，恢复各 Client 端和 CacheServer 端的状态，以继续进行后续服务。

### 3.1.2 可能的解决方案

​        对于 CacheServer，若其发送心跳失败，则认为 Master 已掉线，此后每过一段时间则尝试重新连接到 Master。待 Master 重启后，连接成功，CacheServer 将自身状态发送给 Master 以进行状态恢复。
​        对于 Client，可以采用相似的心跳机制，以监测 Master 的在线状态。同样，Master 掉线后不断尝试重新连接，待 Master 重启后则连接建立成功，恢复正常服务环境。
