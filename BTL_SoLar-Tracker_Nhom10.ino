
#include <Wire.h>
#include <DIYables_LCD_I2C.h>
#include <Servo.h>

// ========================================
//       SOLAR TRACKER - ARDUINO UNO
//         2 LDR + SERVO + LCD I2C
// ========================================

// ===== KHAI BAO LCD =====
DIYables_LCD_I2C lcd(0x27, 16, 2);

// ===== KHAI BAO SERVO =====
Servo servoDoc;

// ===== KHAI BAO CHAN =====
const int LDR_TREN = A0;
const int LDR_DUOI = A1;
const int SERVO_PIN = 10;

// ===== CAU HINH GOC =====
int gocServo = 90;

const int GOC_MIN = 60;
const int GOC_MAX = 120;
const int GOC_MAC_DINH = 90;

// Doi thanh -1 neu servo quay nguoc
const int CHIEU_SERVO = 1;

// ===== CAU HINH ANH SANG =====
const int NGUONG_CAN_BANG = 50;

// Nguong bat dau xac dinh troi toi
const int NGUONG_TOI = 50;

// Nguong thoat che do troi toi
// Cao hon nguong toi de tranh nhap nhay
const int NGUONG_SANG_LAI = 80;

// ===== BO LOC CAM BIEN =====
float ldrTrenLoc = 0;
float ldrDuoiLoc = 0;

// He so loc: 0 < ALPHA <= 1
const float ALPHA = 0.25;

// ===== CHU KY HOAT DONG =====
const unsigned long CHU_KY_DOC = 20;
const unsigned long CHU_KY_LCD = 300;
const unsigned long CHU_KY_SERIAL = 500;

// Troi toi lien tuc 5 giay
const unsigned long THOI_GIAN_TOI = 5000;

// ===== BIEN THOI GIAN =====
unsigned long lanDocCuoi = 0;
unsigned long lanServoCuoi = 0;
unsigned long lanLCDCuoi = 0;
unsigned long lanSerialCuoi = 0;
unsigned long batDauToi = 0;

// ===== BIEN TRANG THAI =====
int huongDiChuyen = 0;
unsigned long thoiGianServo = 25;

bool dangToi = false;
bool cheDoNight = false;

// ========================================
// HAM DOC VA LOC LDR
// ========================================

void capNhatLDR() {
  int tren = analogRead(LDR_TREN);
  int duoi = analogRead(LDR_DUOI);

  ldrTrenLoc += ALPHA * (tren - ldrTrenLoc);
  ldrDuoiLoc += ALPHA * (duoi - ldrDuoiLoc);
}

// ========================================
// HAM KIEM TRA ANH SANG YEU
// ========================================

void kiemTraTroiToi(unsigned long hienTai) {

  int anhSangTB =
    (int)((ldrTrenLoc + ldrDuoiLoc) / 2);

  if (anhSangTB < NGUONG_TOI) {

    if (!dangToi) {
      dangToi = true;
      batDauToi = hienTai;
    }

    if (hienTai - batDauToi >= THOI_GIAN_TOI) {
      cheDoNight = true;
    }

  }
  else if (anhSangTB >= NGUONG_SANG_LAI) {

    dangToi = false;
    cheDoNight = false;
  }
  else {
    // Vung trung gian 50-79
    // Khong xac nhan toi them
    // Neu da vao NIGHT thi tiep tuc giu NIGHT

    if (!cheDoNight) {
      dangToi = false;
    }
  }
}

// ========================================
// HAM XAC DINH HUONG VA TOC DO
// ========================================

void tinhToanDieuKhien() {

  int chenhLech =
    (int)(ldrTrenLoc - ldrDuoiLoc);

  int doLech = abs(chenhLech);

  huongDiChuyen = 0;

  // ===== CHE DO TROI TOI =====
  if (cheDoNight) {

    if (gocServo < GOC_MAC_DINH) {
      huongDiChuyen = 1;
    }
    else if (gocServo > GOC_MAC_DINH) {
      huongDiChuyen = -1;
    }

    thoiGianServo = 25;
    return;
  }

  // ===== ANH SANG YEU =====
  if (dangToi) {
    return;
  }

  int anhSangTB =
    (int)((ldrTrenLoc + ldrDuoiLoc) / 2);

  if (anhSangTB < NGUONG_SANG_LAI) {
    return;
  }

  // ===== VUNG CAN BANG =====
  if (doLech <= NGUONG_CAN_BANG) {
    return;
  }

  // ===== XAC DINH HUONG =====
  if (chenhLech > 0) {
    huongDiChuyen = CHIEU_SERVO;
  }
  else {
    huongDiChuyen = -CHIEU_SERVO;
  }

  // ===== DIEU CHINH TOC DO =====
  if (doLech >= 300) {
    thoiGianServo = 15;  // Nhanh
  }
  else if (doLech >= 150) {
    thoiGianServo = 25;  // Trung binh
  }
  else {
    thoiGianServo = 45;  // Cham, muot
  }
}

// ========================================
// HAM DIEU KHIEN SERVO
// ========================================

void dieuKhienServo(unsigned long hienTai) {

  if (huongDiChuyen == 0) {
    return;
  }

  if (hienTai - lanServoCuoi < thoiGianServo) {
    return;
  }

  lanServoCuoi = hienTai;

  int gocMoi = constrain(
    gocServo + huongDiChuyen,
    GOC_MIN,
    GOC_MAX
  );

  if (gocMoi != gocServo) {

    gocServo = gocMoi;

    servoDoc.write(gocServo);
  }
}

// ========================================
// HAM HIEN THI LCD
// ========================================

void hienThiLCD() {

  // ===== DONG 1 =====
  lcd.setCursor(0, 0);

  if (cheDoNight) {
    lcd.print("NIGHT ");
  }
  else if (dangToi) {
    lcd.print("LOW   ");
  }
  else {
    lcd.print("AUTO  ");
  }

  lcd.print("GOC:");

  if (gocServo < 100) {
    lcd.print("0");
  }

  lcd.print(gocServo);
  lcd.print("  ");

  // ===== DONG 2 =====
  lcd.setCursor(0, 1);

  // Tao chuoi dung 16 ky tu
  char dong2[17];

  snprintf(
    dong2,
    sizeof(dong2),
    "T:%-4d D:%-4d  ",
    (int)ldrTrenLoc,
    (int)ldrDuoiLoc
  );

  lcd.print(dong2);
}

// ========================================
// HAM HIEN THI SERIAL
// ========================================

void hienThiSerial() {

  int chenhLech =
    (int)(ldrTrenLoc - ldrDuoiLoc);

  Serial.print("LDR TREN: ");
  Serial.print((int)ldrTrenLoc);

  Serial.print(" | LDR DUOI: ");
  Serial.print((int)ldrDuoiLoc);

  Serial.print(" | LECH: ");
  Serial.print(chenhLech);

  Serial.print(" | GOC: ");
  Serial.print(gocServo);

  Serial.print(" | TRANG THAI: ");

  if (cheDoNight) {
    Serial.println("NIGHT");
  }
  else if (dangToi) {
    Serial.println("LOW LIGHT");
  }
  else if (huongDiChuyen > 0) {
    Serial.println("TANG GOC");
  }
  else if (huongDiChuyen < 0) {
    Serial.println("GIAM GOC");
  }
  else {
    Serial.println("CAN BANG");
  }
}

// ========================================
// SETUP
// ========================================

void setup() {

  Serial.begin(9600);

  // Khoi tao cam bien
  ldrTrenLoc = analogRead(LDR_TREN);
  ldrDuoiLoc = analogRead(LDR_DUOI);

  // Khoi tao LCD
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("SOLAR TRACKER");

  lcd.setCursor(0, 1);
  lcd.print("SYSTEM START");

  // Khoi tao servo
  servoDoc.attach(SERVO_PIN);
  servoDoc.write(gocServo);

  delay(1500);

  lcd.clear();

  Serial.println("=== SOLAR TRACKER READY ===");
}

// ========================================
// LOOP
// ========================================

void loop() {

  unsigned long hienTai = millis();

  // ===== DOC LDR =====
  if (hienTai - lanDocCuoi >= CHU_KY_DOC) {

    lanDocCuoi = hienTai;

    capNhatLDR();

    kiemTraTroiToi(hienTai);

    tinhToanDieuKhien();
  }

  // ===== DIEU KHIEN SERVO =====
  dieuKhienServo(hienTai);

  // ===== CAP NHAT LCD =====
  if (hienTai - lanLCDCuoi >= CHU_KY_LCD) {

    lanLCDCuoi = hienTai;

    hienThiLCD();
  }

  // ===== SERIAL MONITOR =====
  if (hienTai - lanSerialCuoi >= CHU_KY_SERIAL) {

    lanSerialCuoi = hienTai;

    hienThiSerial();
  }
}
