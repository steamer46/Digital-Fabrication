# Digital-Fabrication
2026 Digital Fabrication 대회 프로젝트

&nbsp;


# ☁️ Smart Weather & Vision Interactive System
> **YOLOv8 기반 실시간 객체 인지 및 유·무선 하이브리드 통신 제어 스마트 디바이스**

![Python](https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![YOLOv8](https://img.shields.io/badge/YOLOv8-000000?style=for-the-badge&logo=ultralytics&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-E63946?style=for-the-badge&logo=espressif&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![OpenCV](https://img.shields.io/badge/OpenCV-5C3EE8?style=for-the-badge&logo=opencv&logoColor=white)

---
&nbsp;

## 📌 Project Overview
&nbsp;

비전 인지 기술(YOLOv8)과 마이크로컨트롤러(ESP32, Arduino) 제어, 3D 프린팅을 융합한 **스마트 인터랙티브 디바이스**입니다.  
사용자 감지, 실시간 기상 데이터(Open-Meteo API) 수집, LED 매트릭스 시각화, TTS 음성 브리핑을 유·무선 하이브리드 통신 파이프라인으로 연결, 버튼을 통한 스테핑 모터와 펜 작동입니다.

| **최종 시스템 구성** | **실시간 비전 & 인터랙션 구동컷** |

| <img width="450" height="306" alt="image" src="https://github.com/user-attachments/assets/c22b782a-9b88-45b5-97fc-f2d3adace322" />
 | <img width="450" height="600" alt="KakaoTalk_20260818_013352085_01" src="https://github.com/user-attachments/assets/910ad5d1-a9de-4fad-b46d-a85a316701e3" />
|

---
&nbsp;
## 🎬 System Demonstration
&nbsp;

실시간 카메라 인지 -> 위치 기반 강수확률 및 기온 데이터 수집 ->  LED 매트릭스 구름 아이콘 시각화 -> TTS 음성 안내까지 통합 실행 -> 버튼 누를시에 우산탈수 진행되는 구동 영상입니다.

| **실시간 통합 시스템 구동 시연 (Video)** |


https://github.com/user-attachments/assets/db6fbb0d-275c-4efe-8234-85f046232f55


---
&nbsp;
## 📡 System Architecture & Communication Pipeline
&nbsp;

저지연 모니터링과 하드웨어 제어의 안정성을 위해 **유·무선 통신 파이프라인을 분리 융합**했습니다.

### 📶 유·무선 통신 특징
1. **무선 스트리밍 (Laptop -> Tablet):**  
   * YOLOv8n 실시간 인지 화면을 **5 GHz 대역의 독립 무선 네트워크**를 통해 태블릿으로 스트리밍
   * 2.4 GHz 대역의 주파수 간섭을 차단하여 전송 끊김 해결.

2. **유선 제어 파이프라인 (Laptop -> ESP32):**  
   * YOLOv8n 검출 패킷을 ESP32 보드로 전송하기 위한 유선 Serial 통신 연결.
   * **115200 bps Baud Rate**로 동기화하여 실시간 패킷 손실 차단.


---
&nbsp;

## 💻 Software Troubleshooting
&nbsp;

| No. | 구분 | 문제 상황 (Issue) | 원인 (Cause) | 해결 방안 (Solution) |

| **1** | **TTS** | 구글 TTS 라이브러리 실행 오류 | 라이브러리 버전 불일치 | **Microsoft Speech API** 기반 음성 엔진으로 대체 적용 |

| **2** | **OpenCV** | 내장 웹캠이 호출되는 현상 | 카메라 인덱스가 기본값 `0`으로 지정됨 | 외장 카메라 인덱스인 **`1`번**으로 변경하여 정상 캡처 처리 |

| **3** | **Motor Logic** | 아두이노 및 드라이버 발열 심화 | 모터 정지 상태에서도 전압이 지속 인가됨 | 회전하지 않는 대기 구간에서는 **전압 인가 차단(Disable)** 제어 로직 적용 |

| **4** | **Serial** | ESP32 통신 데이터 패킷 깨짐 | 통신 속도를 9600 bps로 설정하여 지연 발생 | 양측 통신 속도를 **115200 bps**로 상호 동기화 |

| **5** | **Network** | 음성/데이터 스트리밍 끊김 현상 | 2.4 GHz 와이파이 대역의 주파수 간섭 발생 | 대역폭이 넓고 간섭이 적은 **5 GHz 독립 대역**으로 전환 |

---
&nbsp;
## 🔌 Hardware Troubleshooting

### 1. 케이블 연결 유격 및 단선 방지 (Soldering)
* **문제:** 점퍼 케이블(M/F) 연결부 유격으로 인해 기기 진동 시 신호 끊김 현상 발생.
* **해결:** 피복을 벗긴 후 직접 **납땜(Soldering)** 및 수축 튜브 작업을 진행하여 물리적 연결 신뢰성 확보.

### 2. 모터 드라이버 교체 (L298N $\rightarrow$ A4988)
* **문제:** DC 모터용 드라이버(L298N)로 스테핑 모터를 제어하면서 극심한 발열과 모터 떨림(Jittering) 발생.
* **해결:** 스텝 제어 전용 **A4988 드라이버**로 교체하여 정밀한 각도 및 위치 제어 구현.
  
> 💡 **Motor Driver Comparison:**
> * `L298N`: DC 모터의 높은 토크 및 정/역방향 속도 제어용
> * `A4988`: 스테핑 모터의 정밀한 각도 및 위치 미세 제어용

### 3. 전원 분배 과열 해결 (Buck Converter 적용)
* **문제:** 스텝 모터 전원(12V 2A)을 아두이노 UNO 전원 단자에 직접 바이패스하여 내장 레귤레이터 과열 발생.
* **해결:** **강압 벅 컨버터(Buck Converter)**를 배치하여 전압을 **5V**로 감압 후 아두이노에 공급.

### 4. ESP32 Logic(3.3V)과 LED Power(5V) 간 전압차 보정
* **문제:** ESP32의 3.3V 제어 신호와 LED의 5V 전원 레벨 차이로 인해 LED 색상이 비정상 출력되는 오류 발생.
* **해결:** LED 전원 측 입력 전압을 벅 컨버터로 미세 조율하여 **최적 동작 전압인 4.2V**로 설정, 신호 인식률 극대화.

---
&nbsp;

## 🛠️ Hardware & Circuit Fabrication
&nbsp;
*ESP32, 아두이노, 벅컨버터, LED 매트릭스 통합 배선*
&nbsp;
| **내부 전원 & 제어 회로 배선** |

| <img width="450" height="600" alt="KakaoTalk_20260818_013352085_02" src="https://github.com/user-attachments/assets/ac023553-5baa-4b72-98dd-02b5c9afd513" /> |
&nbsp;
&nbsp;
&nbsp;
&nbsp;

| **3D 프린팅 하우징 제작** | **Wokwi 펌웨어 시뮬레이션** |

|  <img width="450" height="800" alt="KakaoTalk_20260818_040254419" src="https://github.com/user-attachments/assets/4e0ad1c1-2f8d-4deb-b447-3102ff72e99f" /> 
| <img width="450" height="240" alt="KakaoTalk_20260818_013352085_08" src="https://github.com/user-attachments/assets/444b0194-0342-4087-b27a-e590f77eba2e" /> |

| *레트로 TV 스타일 모니터 & 메인 하우징 설계* | *ESP32 + ILI9341 LCD + Open-Meteo API 검증* |

---
&nbsp;

## 🎯 Custom Dataset & Model Training
&nbsp;

* **YOLOv8 Vision Model:**  
  * 카메라 입력에서 인원(`person`) 및 객체를 실시간 검출.
 ---

 


