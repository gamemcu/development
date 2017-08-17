from Tkinter import *
from img2oled import gencode,getmono,decode0

class point:
    def __init__(self,cv,x,y,w,color):
        self.cv=cv
        self.x0=x*w
        self.y0=y*w
        self.x1=self.x0+w
        self.y1=self.y0+w
        self.id=None
        self.color=None
        self.update(color)

    def update(self,color):
        if self.color != color:
            if self.id != None:
                self.cv.delete(self.id)
            self.id=self.cv.create_rectangle(self.x0,self.y0,self.x1,self.y1,fill=color)
            self.color=color

class img:
    def __init__(self,w,h,data):
        self.w=w
        self.h=h
        self.data=data

    def setpixel(x,y,val):
        self.data[y*self.w+x]=val

    def copy(self):
        data=[]
        for v in self.data:
            data.append(v)
        return img(self.fp,self.w,self.h,data)
        
        
class img2oledgui(Frame):
    def __init__(self, parent, **option):
        Frame.__init__(self, parent)
        self.fp=None
        self.color='white'
        self.bakgrd='blue'
        self.pointsize=10
        self.cv=Canvas(parent)
        self.cv.bind('<ButtonPress-1>',self.onpress)
        self.cv.bind('<ButtonRelease-1>',self.onrelease)
        self.cv.width=None
        self.cv.height=None
        self.cv.pack()
        self.workimg=img(None,None,None)
        Button(parent,text='gen',command=self.onclick_gen).pack()
        Button(parent,text='clear',command=self.onclick_clear).pack()

    def onclick_gen(self):
        data=self.wokeimg.data
        for i in range(self.wokimg.data):
            self.workimg.data[i]=0xff-self.wokeimg.data[i]
        self.drawmono(0,h,self.workimg)
        w,h,data=decode0(self.workimg.fp,self.workimg.w,self.workimg.h,self.data)
        print gencode(self.workimg.fp,w,h,data)

    def onclick_clear(self):
        pass

    def onpress(self,event):
        w=self.pointsize
        x=event.x/w
        y=event.y/w
        if (x >= self.workimg.width) or (y >= self.workimg.height):
            return 
        color=self.points[x][y].color 
        if color == self.color:
            color=self.bakgrd
            self.workimg.setpixel(x,y,0xff)
        else:
            color=self.color
            self.workimg.setpixel(x,y,0)
        self.points[x][y].update(color)

    def onrelease(self,event):
        pass

    def drawpoint(self,x,y,color):
        if self.points[x][y] is None:
            self.points[x][y]=point(self.cv,x,y,self.pointsize,color)
        else:
            self.points[x][y].update(color)

    def drawmono(self,x0,y0,img):
        if  (self.workimg is None) or \
            (self.workimg .w != img.w) or\
            (self.workimg .h != img.h):
            self.cv.config(width=img.w*self.pointsize,heigh=2*img.h*self.pointsize)
            self.points=[None]*img.w
            for i in range(img.w):
                self.points[i]=[None]*img.h*2

        for y in range(img.h):
            for x in range(img.w):
                if img.data[y*img.w+x]:
                    self.drawpoint(x0+x,y0+y,self.color)
                else:
                    self.drawpoint(x0+x,y0+y,self.bakgrd)

    def open(self,fp,size=None):
        self.fp=fp
        w,h,data=getmono(fp,size)
        self.drawmono(0,0,img(w,h,data))


def main():
    root=Tk()
    root.geometry("500x500+300+300")
    app=img2oledgui(root)
    app.open('/home/regocxy/myspace/fxmold/fun/assets/kidSprite_12x16.png')
    root.mainloop()  

if __name__ == '__main__':
    main()  
        