# Hash function explain

## Content

- [Designations](#general-designations)
- [FNV hash](#fowlernollvo-hash-fnv)
- [PJW hash](#pjw-hash)
- [SipHash](#siphash-function)

## General designations

Operators:
- `:=` - assign value
- `@=` - apply operator `@` and assing result (`a @= b` equal `a := a @ b`)
- `||` - concatenation two byte sequences
- `|seq|` - length of sequence
- `+`, `-`, `*`, `/`, `%` - addition, subtraction, multiplication, integer division and remainder of the division by modulo 2 to the power bits in integet
- `~`, `&`, `|`, `^` - bitwise NOT, AND, OR and XOR
- `=`, `!=` - logical EQUAL and NOTEQUAL
- `<<`, `>>` - bitwise shift left/right
- `<<<`, `>>>` - bitwise rotate left/right

Types:
- `uint` - generic type for unsigned integers
- <code>u<i>X</i></code> - unsigned integer with width *X*
- `usz` - unsigned size type (usually equal `uint` with max width)
- <code><i>T</i>[<i>N</i>]</code> - array with length *N* and elements with type *T*
- <code><i>T</i>[]</code> - array of elements with type *T*

## Fowler–Noll–Vo hash (FNV)

FNV-1 hash function:
<pre><code><b><i>algorithm</i></b> fnv-1(message: bytes) <b><i>is</i></b>
    uint hash := <i>FNV-offset</i>
    <b><i>for-each</i></b> byte <b><i>in</i></b> message <b><i>do</i></b>
        hash *= <i>FNV-prime</i>
        hash ^= byte
    <b><i>return</i></b> hash
</code></pre>

FNV-1a hash function:
<pre><code><b><i>algorithm</i></b> fnv-1a(message: bytes) <b><i>is</i></b>
    uint hash := <i>FNV-offset</i>
    <b><i>for-each</i></b> byte <b><i>in</i></b> message <b><i>do</i></b>
        hash ^= byte
        hash *= <i>FNV-prime</i>
    <b><i>return</i></b> hash
</code></pre>

Constants:
| Bits | *FNV-offset*       | *FNV-prime*   |
| :--: | :----------------- | :------------ |
|  32  | 0x811c9dc5         | 0x1000193     |
|  64  | 0xcbf29ce484222325 | 0x100000001b3 |

> [!TIP]
> See other size constants on [Wikipedia](https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function#FNV_hash_parameters)

## PJW hash

PJW hash function:
<pre><code><b><i>algorithm</i></b> pjw(message: bytes) <b><i>is</i></b>
    uint high, hash := 0
    <b><i>const</i></b> usz bits := <i>size hash in bits</i>
    <b><i>for-each</i></b> byte <b><i>in</i></b> message <b><i>do</i></b>
        hash := (hash << bits/8) + byte
        high := <i>last</i> bits/8 <i>bits of hash from left</i>
        <b><i>if</i></b> high != 0 <b><i>do</i></b>
            hash ^= (high >> bits * 3/4)
            hash &= ~high
    <b><i>return</i></b> hash
</code></pre>

## SipHash function

SipHash-*c*-*d* function:
<pre><code><i><b>comment:</b> all integer in little-endian</i>
<b><i>algorithm</i></b> siphash-c-d(c: usz, d: usz, key: u128, message: bytes) <b><i>is</i></b>
    <b><i>const</i></b> u64 k0 := <i>lower part of key</i>
    <b><i>const</i></b> u64 k1 := <i>higher part of key</i>

    u64 v0 := k0 ^ 0x736f6d6570736575
    u64 v1 := k1 ^ 0x646f72616e646f6d
    u64 v2 := k0 ^ 0x6c7967656e657261
    u64 v3 := k1 ^ 0x7465646279746573

    <b><i>const</i></b> u64[] blocks := message || <i>padding with zero bytes length = </i>(7 - |message| % 8) || |message| % 256
    <i><b>comment:</b> last block (0-7 bytes) contains length of message by modulo 256 in higher byte</i>
    <i><b>comment:</b> as example: last and only one block for message 'AB' equal 0x0200000000004241</i>

    <b><i>for-each</i></b> block <b><i>in</i></b> blocks <b><i>do</i></b>
        v3 ^= block
        <b><i>repeate</i></b> c <b><i>times</i></b>
            sipround()
        v0 ^= block
    
    v2 ^= 0xff
    <b><i>repeate</i></b> d <b><i>times</i></b>
        sipround()
    
    <b><i>return</i></b> v0 ^ v1 ^ v2 ^ v3
</code></pre>

<pre><code><b><i>algorithm</i></b> sipround() <b><i>is</i></b>
    v0  +=  v1; v2  +=  v3
    v1 <<<= 13; v3 <<<= 16
    v1  ^=  v0; v3  ^=  v2
    v0 <<<= 32
    v2  +=  v1; v0  +=  v3
    v1 <<<= 17; v3 <<<= 21
    v1  ^=  v2; v3  ^=  v0
    v2 <<<= 32
</code></pre>

> [!NOTE]
> Usually use version SipHash-2-4, where *c* = 2 and *d* = 4