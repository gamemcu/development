#!/usr/bin/env python

import sys
import argparse
try:
    import Image,ImageDraw
except Exception as e:
    from PIL import Image,ImageDraw

import glob,os,re
from os.path import basename,splitext

def getsize(name,w,h):
    r=re.findall(r'(\d+)x(\d+).',name) or [(w,h)]
    return int(r[0][0]),int(r[0][1])

def gencode(name,frames,color,w,h,data):
    s = str.format('%s = string.char(0x%02x,0x%02x,0x%02x,0x%02x,\n'%(name,ew,eh,frames,color))
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

def decode0(w,h,data):
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
                if not (data[kk+x][3] == 255 and data[kk+x][0] == 0):
                    v|=1<<i
                kk+=w
            ret.append(v)
        y+=8
    return ret

def decode2(w,h,data):
    ret=[]
    y=0
    imax=8
    while y<h:
        k=y*w
        if y+8>h:
            imax=y&7
        for x in range(w):
            kk=k
            v=0
            v1=0
            for i in range(imax):
                if data[kk+x][3] == 0:
                    v1|=1<<i
                elif data[kk+x][0] == 255:
                    v|=1<<i
                kk+=w
            ret.append(v)
            ret.append(v1)
        y+=8
    return ret

DRAW_NORMAL = 1 << 0
DRAW_MASK = 1 << 1
DRAW_REVERSE = 1 << 2
DRAW_TRANSPARENCY = 1 << 3
DRAW_GRAY = 1 << 4

def decode3(w,h,data,mode='RGBA'):
    color = DRAW_NORMAL
    for d in data:
        if mode == 'RGBA' and d[3] == 0:
            color |= DRAW_MASK
        elif (d[0] > 0) and (d[0] < 255):
            pass
            # color |= DRAW_GRAY
    ret=[]
    y=0
    imax=8
    while y<h:
        k=y*w
        if y+8>h:
            imax=h&7
        for x in range(w):
            kk=k
            v, v1, v2=0, 0, 0
            for i in range(imax):
                if mode == 'RGBA' and data[kk+x][3] == 0: #mask
                    v2|=1<<i
                    v|=1<<i
                elif data[kk+x][0] == 255: #white
                    v|=1<<i
                elif data[kk+x][0] > 0: #gray
                    v1|=1<<i
                kk+=w
            ret.append(v)
            if color & DRAW_GRAY:
                ret.append(v1)
            if color & DRAW_MASK:
                ret.append(v2)
        y+=8
    return ret, color

def getmono(fp,size=None):
    im = Image.open(fp)
    if im.mode=='RGBA':
        ret,ret1=decode2(im.size[0],im.size[1],list(im.getdata()))
    if size != None:
        im.thumbnail(size, Image.ANTIALIAS)
    return im.size,list(im.getdata())

class FileNameListAction(argparse.Action):
    def __init__(self, option_strings, dest, nargs='+', **kwargs):
        super(FileNameListAction, self).__init__(option_strings, dest, nargs, **kwargs)

    def __call__(self, parser, namespace, values, option_string=None):
        mask_list = []
        for v in values:
            mask_list.append(v)
        setattr(namespace, self.dest, mask_list)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='tanslate png to oled bitmap format')
    parser.add_argument('-s', '--src',    default=None,         help='srcource directory of input files ')
    parser.add_argument('-d', '--dest',   default=None,         help='destination directory of output file')
    parser.add_argument('-m', '--mode',   default=1,            help='mode of decode, default is 0, 0:row, 1: column')
    parser.add_argument('-p', '--prefix', default='asset_',    help='prefix of generated code')
    parser.add_argument('-g', '--debug',  action='store_false', help='debug, default is true')
    parser.add_argument('-l', '--list',   default=[], metavar='<filename>', help='need mask filename list, separated by space',action=FileNameListAction)
    args = parser.parse_args()
    if args.src is None:
        sys.stderr.write('you must input the srcource directory of input files\n')
        sys.exit(0)
    if os.path.isdir(args.src):
        args.src = os.path.join(args.src,'*')
    for name in glob.glob(args.src):
        if not name.endswith('.png'):
            continue
        im = Image.open(name)
        w,h=im.size
        ew,eh=getsize(name,w,h)
        frames=h/eh
        name=basename(name)
        func_name,ext=splitext(basename(name))
        func_name=func_name.replace('-', '_')
        data = list(im.getdata())
        # if (name in args.list) and (im.mode=='RGBA'):
        #     ret=decode2(w,h,data)
        # else:
        ret, color=decode3(w,h,data,im.mode)
        s=gencode(args.prefix+func_name,frames,color,w,h,ret)
        print s
