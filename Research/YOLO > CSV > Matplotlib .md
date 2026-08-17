안녕하세요!

말출전까지 Github에다가 뭐든 쓰고 나가야 할거 같아서 요새 AI 공부만 하느라 정신없는 1인 입니다 


​

오늘은 이론만 위주로 YOLO, CSV, Matplotlib만 알아보겠습니다

​

​

과정은 다음과 같습니다

Yolo 추론

### 1. text
 frame, time, count, avg, conf

### 2. yolo > csv로 저장
  eg) df.to_csv( "people_count.csv", index = False)

### 3. pandas로 다음단계
 csv를 다시 불러와서

 eg) df = pd.read_csv( "people_count.csv")

### 4. matplotlib으로 확인​

사람수 변화 그래프

혼합구간 강조표시

### 5. ML 파이브 라인​

> 여기부분은 Yolo 다음 ML단계라서 일단 Yolo 구현을 우선적으로 할것.

## Yolo > csv로 변환

### 1. YOLO
  results =model(img)  #result의 경우에는 이름 변경가능, 배열이라고 생각하기
  r=results[0]
  boxes= r.boxes

​
### 2. tensor -> 파이썬 값으로 바꾸기​

 Yolo결과는 torch tensor라서 그대로 CSV로 못씀
 # GPU에 있을수도 있고, CPU에 있을수도 있기 때문에

​
cls = boxes.cls.cpu().numpy()
conf = boxes.conf.cpu().numpy()
xyxy = boxes.xyxy.cpu().numpy()
이제 numpy 배열

### 3. 한 객체씩 풀어서 딕셔너리로 만들기​

rows = []
for i in range(len(cls)):
   rows.append({
      "class_ id" : int (cls[:]),
      "class_name": r.names[int(cls[:])],
      "confidence" : float(conf[:]),
      "x1" : float(xyxy[:][0]),
      "y1" : float(xyxy[:][1]),
      "x2" : float(xyxy[:][2]),
      "y2" : float(xyxy[:][3]),
})

​
### 4. pandas 로 csv 저장​

import pandas as pd

df= pd.DataFrame(rows)
df. to_csv("people.csv", index= False)

​
## csv > matplotlib로 변환​

csv파일 (people. csv) 있다고 가정


import pandas as pd
import matplotlib.pyplot as plt

​

​

### 1. csv읽기
​
 df=pd.read_csv("people.csv)
 
### 2. csv 내용 확인​

  print(df) or df.head()

컬럼 이름이 정확한지 확인하려고 (이름 틀리면 에러남)

​
### 3. 그래프에 쓸 X,Y 정하기
\
X=df["timestamp"]
Y=df["people_count"] 
​

X축데이터 -> Time stamp
Y축 데이터 -> poeple _count

​
### 4. 선 그래프 그리기​

plt.plot(x,y)

5. 그래프 꾸미기​

plt.xlabel("Time(seconds)")
plt.ylabel("people count")
plt. title("people count over time")
​

## 주의

 문자열 CSV를 숫자로 착각할 수 도 있음 숫자인데 object면
df["people_count"] = df["people_count] as type(int)
타입 바꾸기 필요

​

​추가적으로 제일 중요한

### YOLO(You Only Look Once)란 무엇인가?

object detection은 이미지나 영상속에서 물체가 어디 있는지(위치) 와 무엇인지(클래스) 를 동시에 찾아내는 기술

​
즉, 이 전에 배웠던
​
### 인공신경망에서 CNN(합성곱 신경망) 기반의 객체 탐지 딥러닝 모델 입니다!​

​
이미지를 5*5 그리드로 나눔
각 그리드 셀의 역할


### 1.Bounding Box 좌표 (x,y,w,h)​

 - (x, y) : 물체 중심 좌표
 - (w,h): 물체의 너비와 높이


### 2. confidence(확률)​

 - 그 박스안에 물체가 있을 호가률
​

### 3. class probability​

- 예를 들면 고양이일 확률 , 강아지일 확률, 사람일 확률 등등...




### Q. 그러면 YOLO 버전들은 뭐가 있나요?​

여러개가 있지만 보통
YOLOv5 YOLOv8 이 두개 사용합니다

### YOLOv5: 

속도 빠름, 구조가 단순하나 정확도가 떨어짐
> 드론이나 CCTV 실시간 감시에 용이

​
### YOLOv8:

정확도 높음, 학습/추론 코드가 깔끔 but 속도 느림
> 연구, 산업용 데이터 분석에 용이

​
코드를 작성하면,

<img width="625" height="1391" alt="image" src="https://github.com/user-attachments/assets/4130ec6b-6a74-4474-bb79-d3edd388778c" />

이런식으로 나옵니다
참고로 클래스 번호(cls)에서

0=person
1=bicycle
2=car 이고

xyxy는 박스 좌표, conf는 신뢰도 입니다

일단 간단하게 코드 소개는 여기까지 입니다
다음 에는 이 코드를 가지고 직접 웹캠 혹은 영상에다가 실시간 탐지를 해보려고 합니다

​

​

네

다음번이 메인입니다

​

​

뭔가 머리속에 있는거 그냥 쫙 적는다고 중구난방으로 적은거 같은데

양해 부탁드립니다..
