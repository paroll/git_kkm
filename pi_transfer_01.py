import ftplib
import time
import os

pwd='./images'
while True:
        for path,dirs,files in os.walk(pwd):
                if len(files)<50:
                        break
                try:
                    piftp=ftplib.FTP('118.39.224.204','paroll','4rfvvfr4')
                except:
                    print("connecting error")
                    break
                print("connect")
                number=[]
                for file in files:
                        file=file[:-4]
                        number.append(int(file))
                number.sort()
                num=number[0]
                piftp.cwd('py_kkm/images')
                for i in range(40):
                        upfile='images/'+str(num+i)+'.png'
                        upfilename=str(num+i)+'.png'
                        with open(upfile,'rb') as f2u_bin:
                                piftp.storbinary('STOR %s' %upfilename,f2u_bin)
                        os.remove(upfile)
                piftp.close()
                print("data transfered")
        print("60seconds sleep begin")
        time.sleep(60)

