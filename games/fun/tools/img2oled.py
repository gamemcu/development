import Image,ImageDraw,glob,os,re
from os.path import basename,splitext

def getsize(name,w,h):
    r=re.findall(r'(\d+)x(\d+).',name) or [(w,h)]
    return int(r[0][0]),int(r[0][1])

def gencode(name,frames,w,h,data):
    s = str.format('%s = string.char(0x%02x,0x%02x,0x%02x,\n'%(name,ew,eh,frames))
    rlen = len(data)
    line = 0
    for i in range(0, rlen):
        s += str.format("0x%02x," %data[i])
        if i == rlen-1:
            break
        if i % 12 ==11:
            line += 1
            if line == 8:
                line = 0
                s = s[0:len(s)-1]
                s += ')..string.char(\n'
            else:
                s += '\n'
    s = s[0:len(s)-1]
    s += ')\n\n'
    return s

def decode0(name,w,h,data):
    ew,eh=getsize(name,w,h)
    if h>eh:
        w,h,data=rearrange(w,h,ew,eh,data)
    v=0
    ret=[]
    for y in range(h):
        for x in range(w):
            xx=x&7
            if data[y*w+x]>0:
                v|=1<<xx
            if (xx==7) or (x==w-1):
                ret.append(v)
                v=0
    return ret

def draw(w,h,data):
    k=0
    for y in range(h):
        for x in range(w):
            if data[k+x][0]:
                print('O'),
            else:  
                print('.'),
        k+=w
        print

def decode1(w,h,data):
    ret=[]
    y=0
    imax=8
    while y<h:
        k=y*w
        if y+8>h:
            imax=h&7
        for x in range(w):
            kk=k
            v=0
            for i in range(imax):
                v|=(data[kk+x][0]&1)<<i
                kk+=w
            ret.append(v)
        y+=8
    return ret

def decode2(w,h,data):
    ret=[]
    y=0
    while y<h:
        for x in range(w):
            v=0
            v1=0xff
            k=y*w
            imax=8
            if y+8>h:
                imax=y&7
            for i in range(imax):
                if data[k+x][3]:
                    v|=(data[k+x][0]&1)<<i
                    v1&=~(1<<i)
                k+=w
            ret.append(v)
            ret.append(v1)
        y+=8
    return ret

def rearrange(w,h,ew,eh,data):
    ret=[]
    n=w*eh
    nw=w/ew
    nh=h/eh
    for k in range(nh):
        for i in range(nw):
            for y in range(eh):
                for x in range(ew):
                    ret.append(data[y*w+x+i*ew+k*n])
    w=ew
    h=nw*eh*nh
    return w,h,ret

def getmono(fp,size=None):
    im = Image.open(fp)
    if im.mode=='RGBA':
        ret,ret1=decode2(im.size[0],im.size[1],list(im.getdata()))
    if size != None:
        im.thumbnail(size, Image.ANTIALIAS)
    return im.size,list(im.getdata())

if __name__ == '__main__':
    # fp='/home/regocxy/myspace/fxmold/fun/assets/*.png'
    # fp='/home/regocxy/myspace/fxmold/fun/assets/selector.png'
    # fp='/home/regocxy/Downloads/mainMenu.png'
    fp='/home/regocxy/Downloads/lizi.png'
    prefix='asset_'
    needmask={}
    needmask={'selector.png':1,'kidSprite_12x16.png':1,'balloon.png':1}
    for name in glob.glob(fp):
        im = Image.open(name)
        w,h=im.size
        ret=[]
        ew,eh=getsize(name,w,h)
        frames=w/ew
        frames2=h/eh
        if frames<frames2:
            frames=frames2
        name=basename(name)
        func_name,ext=splitext(basename(name))
        func_name=func_name.replace('-', '_')
        if needmask.get(name) and (im.mode=='RGBA'):
            # func_name+='_withmask'
            ret=decode2(w,h,list(im.getdata()))
        else:
            ret=decode1(w,h,list(im.getdata()))
        s=gencode(prefix+func_name,frames,w,h,ret)
        print s
