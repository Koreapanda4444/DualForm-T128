"""Independent mathematical oracle. Does not load or execute the C library."""
import hashlib
import struct
M64=(1<<64)-1
M32=(1<<32)-1
S=(12,5,6,11,9,0,10,13,3,14,15,8,4,7,1,2)

def rol(x,n,w=64):
    n%=w
    return ((x<<n)|(x>>((w-n)%w)))&((1<<w)-1)

def even(x):
    y=sum(S[(x>>(4*i))&15]<<(4*i) for i in range(16))
    return y^rol(y,11)^rol(y,37)

def odd(x):
    a,b=x&M32,x>>32
    a=(a+rol(b,7,32))&M32
    b^=rol(a,11,32)
    a^=rol(b,17,32)
    b=(b+rol(a,5,32))&M32
    return a|(b<<32)

def shake(domain,data,n):
    return hashlib.shake_256(domain.encode('ascii')+b'\0'+data).digest(n)

def sample(label,j,r,i,n):
    return shake('DualForm-T128/test/'+label,struct.pack('<QIQ',j,r,i),n)

def encode(key,tweak,inverse=False):
    l,r=struct.unpack('<QQ',tweak)
    for i in (range(5,-1,-1) if inverse else range(6)):
        h=int.from_bytes(shake('DualForm-T128/tweak-preperm',bytes([i])+key+struct.pack('<Q',l if inverse else r),8),'little')
        l,r=(r^h,l) if inverse else (r,l^h)
    return struct.pack('<QQ',l,r)

def schedule(key,tweak):
    b=struct.unpack('<24Q',shake('DualForm-T128/base',key,192))
    d=struct.unpack('<24Q',shake('DualForm-T128/derived',key+tweak,192))
    e=struct.unpack('<QQ',encode(key,tweak))
    return tuple(b[i]^(e[i] if i<2 else d[i]) for i in range(24))

def encrypt(sk,plain,q=24,trace=False):
    l,r=struct.unpack('<QQ',plain)
    states=[plain.hex()]
    for i in range(q):
        l,r=r,l^(even if i%2==0 else odd)(r^sk[i])
        states.append(struct.pack('<QQ',l,r).hex())
    return states if trace else struct.pack('<QQ',l,r)

def decrypt(sk,cipher,q=24):
    l,r=struct.unpack('<QQ',cipher)
    for i in range(q-1,-1,-1):
        l,r=r^(even if i%2==0 else odd)(l^sk[i]),l
    return struct.pack('<QQ',l,r)

def selftest():
    cases=[(bytes(32),bytes(16),bytes(16),'3a6831014e21bb35e87d4e4e458ddda0'),
           (bytes(range(32)),bytes(range(16)),bytes(range(16)),'cab683110ef545617d4e892a779daacf'),
           (b'\xff'*32,b'\xff'*16,b'\xff'*16,'d653cb3700ff90e79631907cbebe9d8e')]
    for k,t,p,c in cases:
        sk=schedule(k,t)
        assert encrypt(sk,p).hex()==c
        assert decrypt(sk,bytes.fromhex(c))==p
        assert encode(k,encode(k,t),True)==t
        for q in range(25):
            assert decrypt(sk,encrypt(sk,p,q),q)==p
    print('Python oracle: 3 KATs, 75 prefix inverses, tweak inverse passed')

if __name__=='__main__':
    selftest()
