import cv2          # 웹캠 영상 처리, 화면 출력, 텍스트 그리기 (opencv 라이브러리)
import time         # time.sleep(), time.time() 으로 시간 관련 기능 사용
import serial       # C타입 케이블로 esp32와 연결할 때 사용
import threading    # 음성 재생 및 터미널 입력을 백그라운드에서 실행하기 위해 사용 (yolo, opencv용)
import asyncio      # edge_tts가 async/await를 사용하기 때문에 asyncio.run()으로 감싸줘야 함 (tts가 threading에서 이거를 이용)
import subprocess   # 윈도우 기본 플레이어로 mp3 재생할 때 사용
import edge_tts     # 마이크로소프트 신경망 TTS, 자연스러운 한국어 목소리
from ultralytics import YOLO  # YOLOv8 모델 로드

async def _generate(text): #비동기 방식
    communicate = edge_tts.Communicate(text, "ko-KR-InJoonNeural") #text를 읽음 communicate()는 이 텍스트를 이목소리로 읽어줘 라는 객체를 만드는 함수
    await communicate.save("alert.mp3") #변환된 음성이 mp3로 저장될때 까지 기다림

def play_alert(text): 
    try:
        asyncio.run(_generate(text)) #비동기 함수니까 받을때도 비동기 함수로
        subprocess.Popen(['start', 'alert.mp3'], shell=True) #subprocess로 mp3 파일 재생 popen으로 새로운 프로그램 실행 shell=True는 명령프롬프트를 실행한다음에 start alert.mp3를 실행하라는 의미
    except Exception as e: #참고로 start명령어로 인하여 팝업이뜲
        print(f"음성 재생 중 오류 발생: {e}")


def terminal_input_handler(ser): #함수 정의, 시리얼 포트로 명령어를 보내는 역할
    print(">> 터미널 명령어 입력이 가능합니다. (보낼 명령어를 입력 후 엔터)")
    while True:
        try:
            cmd = input().strip()  # ">> " 프롬프트를 빼서 출력 정렬을 깔끔하게 유지(즉 .strip()로 띄어쓰기는 다 무시), input으로 키보드 입력 기다림
            if cmd:
                ser.write((cmd + "\n").encode()) #\n으로 칸을 나눈다. encode로 바이트로 변환 ser.write로 시리얼 포트로 전송
                print(f"[보냄] {cmd}")
        except Exception as e:
            print(f"터미널 입력 전송 중 오류 발생: {e}")
            break

# 1. YOLOv8 경량 모델 로드
model = YOLO("yolov8n.pt") #yolo 버전 8 나노 버전이용 실시간 객체 탐지용으로 이거씀!!!

# 2. 웹캠 열기 (외부 USB 웹캠 사용)
cap = cv2.VideoCapture(0) #0번이 보통 내장 캠 근데 숫자 바꾸면 외장 캠으로도 가능

if not cap.isOpened(): 
    print("오류: 웹캠을 열 수 없습니다.")
    exit()

# [기본 설정] 웹캠에 해상도 요청 (지원 안 할 경우 기본값으로 켜짐)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

# 시리얼 연결 (포트 번호는 환경에 맞게 수정)
try:
    esp32 = serial.Serial('COM7', 115200) #esp32랑 통신, com3로 포트 번호 연결, 115200은 통신 속도
    time.sleep(2) #esp32 부팅까지 좀 기다려야함
    print("ESP32 연결 성공")
except Exception as e:
    print(f"ESP32 연결 실패: {e}")
    exit()

COOLDOWN_SEC = 60 #60초 쿨다운 설정, 사람 감지 후 60초 동안은 다시 신호를 보내지 않음
last_sent_time = 0 
window_name = "YOLOv8 Detection" 

#화면 크기 설정
DESIRED_WIDTH = 1280
DESIRED_HEIGHT = 720

# 터미널 입력 전용 백그라운드 스레드 시작
input_thread = threading.Thread(target=terminal_input_handler, args=(esp32,), daemon=True) #args=(esp32,)로 esp32 객체를 전달, daemon=True로 메인 스레드 종료 시 자동 종료
input_thread.start() #본격적으로 thread 시작

# 미리보기 창 설정 및 초기 창 크기를 가로로 넓게 지정
cv2.namedWindow(window_name, cv2.WINDOW_NORMAL) #opencv창을 만들고 창 크기 자유롭게 조절 가능
cv2.resizeWindow(window_name, DESIRED_WIDTH, DESIRED_HEIGHT) 

print("탐지를 시작합니다. 영상 창을 클릭한 후 'q'를 누르면 종료됩니다.")

while True:
    ret, frame = cap.read() #현재 카메라에서 프레임을 읽어옴 ret은 성공 여부, frame은 읽어온 이미지
    if not ret: 
        print("화면을 가져올 수 없습니다.")
        break

    # 1. ESP32로부터 데이터 수신 처리 (시리얼 수신부)
    try:
        if esp32.in_waiting > 0: #정보가 들어와서 존재한다면,
            line = esp32.readline().decode('utf-8').strip() #decode로 바이트를 사람이 읽는 문자열로 변환, strip로 공백제거
            print(f"\n[ESP32 수신] {line}") 

            if line.startswith("WEATHER:"): #weather로 시작시
                data = line.replace("WEATHER:", "").split(",") #"weather:를 빈문자열 "로 대체하고 쉼표로 대체
                temp = float(data[0])     #실수형 
                rainProb = float(data[1])  #실수형
                isRaining = data[2] #그대로 저장

                # 멘트 조합 규칙 적용
                full_text = (
                    " 사람이 감지 되었습니다 ... "
                    " 안녕하세요 . "
                    f" 현재 위치하고 있으신 마포구 일대 온도는 {temp}도입니다... "
                )

                if temp < 0:
                    full_text += " 날씨가 매우 춥기때문에 패딩과 긴팔을 입어서 몸을 따뜻하게 유지해주세요. "
                elif 0 <= temp < 10:
                    full_text += " 날씨가 춥기때문에 얇은 패딩이나 코트와 긴팔을 입어서 몸을 따뜻하게 유지해주세요. "
                elif 10 <= temp < 20:
                    full_text += " 날씨가 선선하기때문에 긴팔을 입어서 몸을 따뜻하게 유지해주세요. "
                elif 20 <= temp < 30:
                    full_text += " 날씨가 따뜻하기 때문에 반팔을 입어서 몸을 시원하게 유지해주세요. "
                else:
                    full_text += " 날씨가 매우 더우므로 반팔과 반바지를 입어서 몸을 시원하게 유지해주세요. "

                full_text += " ... 마포구 일대 강수 여부를 확인합니다 ... "

                if isRaining == "1":
                    full_text += " 비가 오고 있으므로 우산탈수를 진행후에 챙겨가시는걸 추천드립니다. "
                    print("안내: 비가 오고 있습니다. 우산탈수 기능을 안내합니다.")
                else:
                    full_text += " 비가 오고 있지 않습니다. "
                    print("안내: 비가 오지 않습니다.")

                full_text += f" ... 마포구 일대 강수확률은 {rainProb}%입니다. 7월부터는 장마 기간이므로 우산을 챙기시는걸 추천드립니다. "

                # 음성 재생 스레드 구동
                threading.Thread(target=play_alert, args=(full_text,), daemon=True).start()

    except OSError as e:
        print(f"시리얼 통신 일시 오류: {e}")

    # 2. YOLO 영상 탐지 및 화면 표시
    results = model(frame, classes=[0], conf=0.6, verbose=False) #0번객체인 사람을 찾아줘 신뢰도는 60퍼로 둚
    annotated_frame = results[0].plot() #탐지 결과를 영상에 그려줌

    person_detected = len(results[0].boxes) > 0 
    now = time.time() 
#사람이 있는지 확인하고 현재시간을 적기
    # 사람 감지 시 ESP32로 1 전송 (쿨다운 60초 적용)
    if person_detected and (now - last_sent_time) >= COOLDOWN_SEC: 
        try:
            esp32.write(b'1\n') 
            last_sent_time = now 
            print("\n[이벤트] 사람 감지! ESP32로 신호(1) 전송 완료")
        except Exception as e:
            print(f"ESP32 신호 전송 실패: {e}")

    # 영상 출력 및 키 입력 대기 이거 없으면 영상 안나옴
    cv2.imshow(window_name, annotated_frame) 
    
    if cv2.waitKey(1) & 0xFF == ord('q'): #q를 누르면 종료
        break

cap.release() 
esp32.close() 
cv2.destroyAllWindows()
print("프로그램이 정상 종료되었습니다.")
