# NORVI X1 & X-AQ4 Tutorial

# 0-10 V Analog Output from X-AQ4


## **1. Introduction**


Many drives, valve positioners and process actuators take their set-point as a 0–10 V analog signal. This tutorial shows how to generate that signal with a NORVI X1 controller and a NORVI X-AQ4 four-channel analog output module. AOUT1 is the speed reference of a Schneider Electric Altivar Machine ATV320 drive that runs a three-phase motor, type Y2A-6324.


The tutorial covers the wiring, the programming of the ATV320 parameters for this motor, the Arduino settings, the key parts of the sketch, and a commissioning and test procedure. The same method suits pumps, fans, conveyors and any device with a 0–10 V set-point input.


## **2. System Architecture**


The system forms a short signal chain from the operator’s finger to the motor shaft. The blocks and the data-processing logic are shown below.

```text
NORVI X1 I²C NORVI X-AQ4 0–10 V ATV320U11M2C U/V/W Motor
ESP32-S3, touch TFT ─────────────── AOUT1 ───────────────── AI1 = speed reference ───────── Y2A-6324
```

**Figure 1:** *_Signal path from the X1 touch display to the motor_*


### **2.1 Signal Flow**


1. **Input.** The operator touches “+” or “ − ” (one 0.1 V step per press) or sends a number between 0 and 10 on Serial.


2. **Conversion.** The sketch limits the value to 0–10 V and converts it to a DAC code with `voltageToDAC()`.


3. **Output.** The code is written to channel 1 of the X-AQ4 over I²C. The module turns it into a 0–10 V level on AOUT1.


4. **Drive.** The voltage reaches AI1 of the ATV320. With the settings of Section 4, 0 V gives the low speed (0 Hz) and 10 V gives the high speed (50 Hz).


5. **Indication.** The display shows the commanded voltage, and a status dot turns red if an I²C write fails.

### **2.2 Hardware Components**


|**Component**|**Role**|**Key details**|
|---|---|---|
|NORVI X1 (CPU-S3)|Controller|ESP32-S3, 2.0 in ST7789 TFT (240 × 320) with CST816S capacitive touch, 24 V DC supply|
|NORVI X-AQ4|Analog output|Four analog outputs AOUT1–AOUT4 with AGND returns, I²C slave (address 0x5F in the sketch), mounted on the X1 expansion port [3]. **This tutorial uses AOUT1 only.**|
|Altivar Machine ATV320U11M2C|Motor drive|1.1 kW, single-phase 200–240 V input, 6.9 A output, AI1 0–10 V (30 kΩ, 10 bit), STO input [2]|
|Motor Y2A-6324|Load|Three-phase induction motor. The terminal-box label shows 240 V (delta) and 415 V (star)|

**Table 1:** *_Hardware components used in the 0–10 V analog output system_*


### **2.3 Pin Allocation**


|**Function**|**GPIO**|**Notes**|
|---|---|---|
|I²C (SDA / SCL)|IO8 / IO9|Carries the X-AQ4 and the touch controller. Set with `Wire.begin(SDA_PIN, SCL_PIN)`|
|SPI (MOSI / MISO / SCLK)|IO11 / IO13 / IO12|Display bus, configured in the `LGFX` class|
|TFT (CS / DC / RST)|IO45 / IO46 / IO47|ST7789 panel, driven through LovyanGFX [4]|
|Touch (SDA / SCL / RST / INT)|IO8 / IO9 / IO47 / IO48|CST816S, `CST816S touch(8, 9, 47, 48)` [5]|
|X-AQ4 address|0x5F|`AQ4_ADDR` in the sketch. The module must answer at this address|

**Table 2:** *_GPIO and interface pin allocation for the NORVI X1 and X-AQ4_*


## **3. Hardware and Wiring**


### **3.1 Connect the Modules**


1. Switch off the mains and the 24 V supply.


2. Set the X-AQ4 address switches so that the module answers at 0x5F, the value in the sketch (see the X-AQ4 datasheet [3]).


3. Mount the X-AQ4 on the DIN rail beside the X1 and seat it firmly on the expansion connector.


4. Wire AOUT1 (terminal 1) and AGND (terminal 2) to the ATV320, as in Table 3.


5. Wire the ATV320 power and the motor as in Table 4.


6. Connect the 24 V DC supply to the X1 and the USB cable to the PC.

### **3.2 ATV320 Control Wiring**


|**From**|**To (ATV320)**|**Purpose**|
|---|---|---|
|X-AQ4 AOUT1 (terminal 1)|AI1|Speed reference, 0–10 V|
|X-AQ4 AGND (terminal 2)|COM (analog common)|Return of the reference signal|
|P24|STO|Safe torque off input, linked to the drive’s 24 V so that the drive can run|
|P24 via a run switch|DI1|Forward run command (source logic, switch SW1 on Source)|

**Table 3:** *_Control terminals used in this tutorial [2]_*


#### **Run Command**


Wiring DI1 permanently to P24 gives a run command at every power-up. With the factory 2-wire type (transition) the drive then waits for a new edge and shows nSt, as described in Section 4. Use a switch on DI1, so that the operator decides when the motor may start. A permanent link between P24 and STO removes the safe-torque-off function. Accept that only on a test bench, and never in place of an emergency stop.


### **3.3 Power and Motor Wiring**


The motor terminal box carries two terminal rows joined by three brass links. Figure 3 shows the label inside the cover: with the three links in line (240 V) the windings are connected in delta, and with the single star link (415 V) they are connected in star. The ATV320U11M2C is supplied from a single phase of 200–240 V and cannot deliver more than 240 V to the motor, so the motor must stay in the **240 V delta** connection [2].


|**ATV320 terminal**|**Wire**|**Motor terminals it serves**|
|---|---|---|
|R/L1|Line (L)|Single-phase supply, through the breaker / RCD|
|S/L2|Neutral (N)|Single-phase supply|
|PE|Earth|Supply earth and motor earth|
|U/T1|Grey|Link U1 + W2|
|V/T2|Black|Link V1 + U2|
|W/T3|Brown|Link W1 + V2|

**Table 4:** *_Power and motor connections. Wire colours as read from the motor in Figure 3; verify them on your motor before connecting_*

![figure2_wiring_diagram]([docs/Images/figure2_wiring_diagram.png](https://github.com/NORVIControllers/NORVI-X-Version-02/blob/main/docs/Images/figure2_wiring_diagram.png)) 
*Figure 2: Wiring: X-AQ4 AOUT1 and AGND to AI1 and COM of the ATV320, 24 V links, mains and motor.*

![figure3_motor_terminal_box](docs/Images/figure3_motor_terminal_box.png)

*Figure 3: Motor terminal box: three links in line (delta), grey, black and brown cable wires.*

Terminal naming on the motor follows IEC 60034-8 [7]. Each wire sits on one link, and a link joins two terminals, so go by the link the wire is on and not by the “2” name alone: the black wire on the U2 terminal belongs to the V link and goes to V. Swapping any two of U, V and W reverses the direction of rotation.




## **4. Programming ATV320 for Motor Y2A-6324**


### **4.1 The Motor and Why the Drive Is Oversized**


Read the nameplate before programming. The designation Y2A-6324 appears to follow the common Y2 naming (frame 63, second length, four poles), which would be a motor of about 0.18 kW. Treat that as a hint, not as data. The last column of the nameplate table shows what a motor of that class typically has, for example 1380 rpm, 0.6 A at 400 V and a power factor of 0.62 in one manufacturer’s data sheet [8]. Enter the values printed on your own motor.


The ATV320U11M2C is a 1.1 kW drive with a rated current of 6.9 A. The programming manual limits the nominal motor current nCr to 0.25 to 1.5 times the drive rating and the motor thermal current ItH to 0.2 to 1.5 times [1], which is about 1.7 A and 1.4 A at the lower end. A small motor with a nameplate current near 1 A therefore cannot be entered exactly. Set the lowest value the drive accepts, and do not rely on the drive for thermal protection of the motor. Fit a motor protection device suited to the motor current.


|**Quantity**|**Code**|**Typical value for the Y2A-6324**|
|---|---|---|
|Rated power|nPr|0.18 kW (4-pole, 63-frame)|
|Rated voltage|UnS|230–240 V (delta)|
|Rated current|nCr|1.0 A at 240 V|
|Rated frequency|FrS|50 Hz|
|Rated speed|nSP|1350–1400 rpm|
|Power factor|COS|0.6–0.7|
|Poles|–|4 (1500 rpm synchronous at 50 Hz)|

**Table 5:** *_Motor nameplate data and typical reference values for the Y2A-6324_*


### **4.2 Entering Parameters on the Drive**


All settings in this section are made on the integrated display of the drive, with the jog dial and the ESC key [1]. Turn the dial to scroll, press it to enter a menu or confirm a value, and press ESC to go back. The motor parameters are in the `CONF` → `FULL` → `SIM-` (Simply start) and `drC-` (Motor control) menus, the input settings in `I-O`, and the command and reference channels in `CtL-`.


On a drive that has been used before, restore the factory settings first ( `CONF` → `FULL` → `FCS-` ). This erases all parameters, so note anything you need to keep. The factory settings already select 2-wire control, the combined profile and AI1 as the speed reference, which is exactly what this tutorial needs [1].


### **4.3 Parameter Settings**


The full list of parameters is in Appendix A. The parameters fall into two blocks:


- **Motor parameters.** Rated power, voltage, current, frequency, speed, power factor, thermal current, and the motor control type. These describe the physical motor and let the drive compute its internal model. Changing any of them resets the drive’s tuning, so enter them in one pass before the first run.


- **Reference and command parameters.** How the drive is told to start and stop (2-wire control on DI1), and how it maps the AI1 voltage onto a speed range (LSP to HSP). These can be set at any time and edited freely without a reset.


Table 8 in Appendix A gives the code, the menu path, the value used in this tutorial, and a

one-line note for every parameter. Read the motor block first, enter every value from your nameplate, then enter the reference block.


### **4.4 Motor Control Type:** `StD`


One parameter in the motor block needs special attention. If the drive’s factory Motor Control Type ( `Ctt` ) is `UUC` (Sensorless Flux Vector Control), the drive injects test pulses on AI1 above 0 V to measure the rotor phase-shift angle. A frame-63 motor such as the Y2A-6324 draws too little current for that measurement to complete reliably, and the drive trips on overcurrent. Change `Ctt` to `StD` (Standard V/Hz) in the `drC-` menu before the first run.


**1.** Navigate to `CONF` → `FULL` → `drC-` (Motor Control menu).


**2.** Scroll to **Motor Control Type** ( `Ctt` ).


**3.** Change it from `UUC` (Sensorless Flux Vector Control) to `StD` (Standard V/Hz two-point law).


**4.** Press the dial once to save. ESC discards the edit.


## **5. X-AQ4 Communication**


### **5.1 Set Up the IDE**


- Select **ESP32S3 Dev Module** , then set USB CDC On Boot to **Enabled** , Flash Size to **16MB (128Mb)** and PSRAM to **QSPI PSRAM** , as for the other X1 tutorials. Without USB CDC On Boot the Serial Monitor stays empty.


- Install **LovyanGFX** [4] and **CST816S** [5] from the Library Manager. Wire and SPI come with the ESP32 core.


- Open the Serial Monitor at 115200 baud with the line ending set to Newline, to type a voltage.


### **5.2 How the Sketch Talks to the X-AQ4**


The module is an I²C slave at `AQ4_ADDR` = 0x5F. The sketch uses two frames, taken from the working code.


|**Frame**|**Bytes**|**Meaning**|
|---|---|---|
|Configuration|`0x01`, `cfg`|`cfg` = c1 *_|_* c2_≪*_1 _*|*_ c3_*≪*_2 _*|*_ c4_*≪_3. The sketch sends `config_dac(0x5F, 0, 0, 1, 1)`|
|Write channel n|`1 + 2n`, high byte, low byte|Register 3 for AOUT1, 5 for AOUT2, 7 for AOUT3, 9 for AOUT4. Value 0–8191. This tutorial writes channel 1 (AOUT1) only.|

**Table 6:** *_I_*<sup>²</sup> C frames used by the sketch


The sketch scales 0–10 V to the code range 0–8191:

\[\mathrm{DAC} = \frac{V}{10} \times 8191, \qquad 1\text{ count} \approx 1.22\text{ mV}.\]


Channel 1 (AOUT1) receives the configuration value 0, which is the setting of the working 0–10 V output. Channels 3 and 4 receive 1 and are not used. Check the meaning of this bit in the X-AQ4 datasheet before you use AOUT3 or AOUT4 [3].

## **6. Commissioning and Test**


### **6.1 Commissioning Checklist**


- **Safety.** The motor is free to turn, with no load that a start could damage. The breaker and the RCD are fitted. Lock-out is applied while you wire.


- **Supply.** The 24 V supply feeds the X1, and the display shows the main screen with a green status dot.


- **Drive.** The motor parameters and Table 8 are entered (including `Ctt = StD` ), and the drive shows rdY when the mains is on.


- **Wiring.** AOUT1 goes to AI1, AGND to COM, P24 to STO, and the run switch to DI1.


- **Run switch.** The DI1 switch is open.


### **6.2 First Run**


1. Set 0 V, close the DI1 switch and check that the drive reads 0 Hz and the motor stays still. If the display shows nSt, open and close the switch once.


2. Raise the output to 1 V. The motor should turn slowly at about 5 Hz. If the motor twitches and the drive trips on overcurrent ( `OCF` ) or short-circuit ( `SCF` ) at 1 V, check that `Ctt` is set to `StD` (Section 4.4) before changing anything else.


3. Check the direction of rotation. If it is wrong, switch off and wait 15 minutes, then swap two of U, V, W.


4. Raise the output in steps to 5 V and then 10 V and compare the frequency on the drive with Table 8.


5. Lower the output to 0 V and open the DI1 switch. The motor should decelerate to rest in the dEC time.


### **6.3 Expected Output**


**Why the ATV Display May Read 2.0 While the X1 Reads 5.8 V** The X1 shows the commanded reference (the voltage it writes to AOUT1). The ATV320 display shows whichever monitoring value is currently selected, and that is not the AI1 input voltage by default. A reading of `2.0` on the drive while the X1 shows `5.80` is almost always one of the following.


- **The display is showing output frequency.** At 5.8 V with LSP = 0 Hz and HSP = 50 Hz the drive runs at about 29 Hz; if the monitor shows `2.0` instead, the ramp is still running or the frequency monitor is selected.


- **AI1 is not the active reference.** If the profile has been changed ( `CHCF` *_̸_* = `SIM` ) or `Fr1` is not `AI1` , the drive ignores the 0–10 V on AI1 and shows a different reference. Check `Fr1 = AI1` and `CHCF = SIM` (Table 8).


- **The X1 and the meter disagree.** Measure AOUT1 to AGND with a multimeter. If the meter reads 5.8 V and the drive reads 2.0 A or 2.0 Hz, the wiring and the DAC are fine and only the display selection needs changing. If the meter reads 2.0 V, the drive is correct and the fault is on the X1 side (see the “AOUT1 reads half the value” row in Table 7).


In short: the X1 shows what it sends, the ATV shows what it monitors. Match the monitor to `AI1` or `FRD` and the two displays will agree within the drive’s 10-bit AI1 resolution.


![figure4_working_setup](docs/Images/figure4_working_setup.png)

*Figure 4: Working setup: NORVI X1 with X-AQ4 (left), ATV320 and the motor (right). The X1 shows AOUT1 at 5.80 V.*

## **7. Troubleshooting**


|**Symptom**|**Likely cause**|**Remedy**|
|---|---|---|
|Serial Monitor empty|USB CDC On Boot disabled or wrong port|Enable USB CDC On Boot, reselect the port, set 115200 baud, press reset|
|Display blank|Wrong board settings or panel pins|Use the settings in Section 5. Keep CS 45, DC 46, RST 47 in the LGFX class|
|Touch does not respond|CST816S pins or library|Check `CST816S touch(8, 9, 47, 48)` and that the library is installed|
|“I2C Write Error: 2”|No module at the address|Reseat the X-AQ4, check its address switches against 0x5F|
|“I2C Write Error: 3” or red dot|Module does not accept the data|Check the supply and the module firmware, then power-cycle|
|AOUT1 reads half the value|Code scale is 4095|Use 8191 for 10 V (Section 5)|
|Voltage correct, drive does not run|No run command, STO open or transition type|Check the DI1 switch and the P24–STO link. Open and close DI1 once if nSt is shown|
|Voltage above 0 V, motor twitches and drive trips (`OCF` / `SCF`)|`Ctt` still on `UUC`; small motor current too low for the phase-angle measurement|Set `Ctt = StD` in `drC-` (Section 4.4)|
|Voltage correct, speed does not follow|Fr1 is not AI1 or the profile was changed|Check `Fr1 = AI1` and `CHCF = SIM`|
|Speed wanders or jitters|AGND not tied to COM, or noise|Connect AGND to COM, use a shielded pair, add a small AI1F filter|
|Motor turns the wrong way|Two phases interchanged|Swap two of U, V, W with the power off|
|Motor current or heating high|Wrong connection or voltage|Check the 240 V delta connection and UnS|

**Table 7:** *_Troubleshooting guide for the X1, X-AQ4 and ATV320_*


#### **Safety**


Disconnect all power before wiring, and let a qualified electrician install the mains, the drive and the motor. This set-up is a bench demonstration. Do not use it as a safety system or as an emergency stop.


## **8. FAQ**


#### **Why not generate the 0–10 V signal directly from the ESP32?**


The ESP32-S3 outputs 0–3.3 V logic levels, and a filtered PWM signal needs level shifting, scaling and filtering to give a stable 0–10 V. The X-AQ4 provides a ready 0–10 V output with a defined resolution and is set over I²C.


#### **Why AI1 and not AI3?**


AI1 is the 0–10 V voltage input of the drive. AI3 is the current input (0–20 mA or 4–20 mA) [2]. Use AI3

only if the X-AQ4 channel is set to current output.


#### **Can the drive be controlled over Modbus instead?**


Yes. The ATV320 has an embedded Modbus port, and the same X1 family can set the run command and the frequency over RS-485 [9]. The analog method needs no protocol set-up and is the simplest to commission, but it carries only the speed reference. Run and stop still need a separate input.


#### **Can I reverse the motor?**


Yes. Wire a second switch between P24 and DI2, which is the reverse input at the factory [1]. Do not reverse a mechanism at speed without a suitable deceleration time.


#### **Does 0 V stop the motor?**


At LSP = 0 Hz the drive output goes to 0 Hz, but the drive stays in the run state. Open the DI1 switch to stop.


#### **Can I use AOUT2, AOUT3 or AOUT4 later?**


Yes. Each channel is written independently with `write_channel(AQ4_ADDR, n, value)` , where n is 1, 2, 3 or 4. The wiring and scaling are the same as AOUT1. Check the configuration value they receive in `config_dac()` , because the sketch currently sends 1 for channels 3 and 4.


#### **Why does the motor twitch and trip on a small motor at 1 V?**


The factory setting of `Ctt` may be `UUC` (Sensorless Flux Vector Control). In this mode the drive injects test pulses to measure the rotor phase-shift angle, which fails on a low-current frame-63 motor. Set `Ctt = StD` (Section 4.4).


## **Appendix A. ATV320 Parameter Settings and Menu Paths**


Table 8 lists every ATV320 parameter used in this tutorial in one place, with its menu path so it can be found on the drive without searching. The motor block must be entered first; the reference block can then be set without further resets.


- Motor parameters: `ESC` → `CONF` → `FULL` → `SIM-` → `bFr / nPr / UnS / nCr / FrS / nSP / ItH`


- Motor power factor (COS) and control type (Ctt): `ESC` → `CONF` → `FULL` → `drC-` → `COS / Ctt`


- Reference and command channels: `ESC` → `CONF` → `FULL` → `CtL-` → `Fr1 / CHCF / tCC / tCt / Cd1`


- Analog input AI1 settings: `ESC` → `CONF` → `FULL` → `I-O` → `AI1-` → `AI1t / UIL1 / UIH1 / AI1F`


- Speed limits and ramps: `ESC` → `CONF` → `FULL` → `SET-` → `LSP / HSP / ACC / dEC`


- Factory reset: `ESC` → `CONF` → `FULL` → `FCS-` → `InI`

|**Code**|**Name**|**Menu path**|**Value**|**Note**|
|---|---|---|---|---|
|**Motor **|**parameters (set first**|**)** |||
|bFr|Motor standard|SIM-|50 Hz|Factory 50 Hz|
|nPr|Nominal motor power|SIM-|Nameplate kW|Factory depends on the drive|
|UnS|Nominal motor voltage|SIM-|240 V|Range 100–240 V on this single-phase drive|
|nCr|Nominal motor current|SIM-|Nameplate A|Lowest accepted value about 0.25 × 6.9 A|
|FrS|Nominal motor frequency|SIM-|50 Hz|Factory 50 Hz|
|nSP|Nominal motor speed|SIM-|Nameplate rpm|If only slip is given, calculate speed from it|
|COS|Motor 1 cosine phi|drC-|Nameplate value|Range 0.5–1|
|tFr|Max frequency|SIM-|60 Hz (factory)|Keep at least 110 % of HSP|
|ItH|Motor thermal current|SIM-|Nameplate A|Lowest accepted value about 0.2 × 6.9 A|
|Ctt|Motor control type|drC-|StD|**Do not leave on the factory** `UUC` on a small frame-63 motor, see Section 4.4|
|**Reference and command parameters**||||
|tCC|2/3-wire control|CtL-|2C|Factory. DI1 = forward, DI2 = reverse|
|tCt|2-wire type|CtL-|trn (factory)|Transition: a new edge on DI1 is needed to start. Use LEL only if a restart at power-up is safe|
|CHCF|Profile|CtL-|SIM (factory)|Reference and command share one channel|
|Fr1|Reference channel 1|CtL-|AI1 (factory)|The 0–10 V from the X-AQ4|
|AI1t|AI1 type|I-O →AI1-|10U (factory)|Voltage input 0–10 V|
|UIL1 / UIH1|AI1 min / max value|I-O →AI1-|0 V / 10 V (factory)|Voltage that gives LSP and HSP|
|AI1F|AI1 filter|I-O →AI1-|0 s (factory)|Optional 0.1–0.5 s smooths 0.1 V steps|
|LSP|Low speed|SET-|0 Hz (factory)|Frequency at 0 V|
|HSP|High speed|SET-|50 Hz|Frequency at 10 V. Lower it to limit rig speed|
|ACC / dEC|Accel / decel ramp|SET-|3 s (factory)|Use 5–10 s for a valve mechanism|

**Table 8:** *_ATV320 parameter settings for the X-AQ4 speed reference. Values checked against the programming manual [1]_*


## **References**


- [1] Schneider Electric, *_Altivar Machine ATV320 Variable Speed Drives for Asynchronous and Synchronous Motors: Programming Manual_* , NVE41295.06, Apr. 2025.


- [2] Schneider Electric, “ATV320U11M2C: Variable speed drive, Altivar Machine ATV320, 1.1 kW, 200 to 240 V, 1 phase, compact,” Product data sheet, May 8, 2026.

- [3] NORVI, “NORVI X-AQ4.” [Online]. Available: `https\://shop.norvi.lk/products/norvi-x-aq4` (accessed Nov. 17, 2026).


- [4] lovyan03, “LovyanGFX: SPI LCD graphics library for ESP32 and other boards,” GitHub repository. [Online]. Available: `https\://github.com/lovyan03/LovyanGFX` (accessed Nov. 17, 2026).


- [5] F. Biego, “CST816S: Arduino library for the CST816S touch controller,” GitHub repository. [Online]. Available: `https\://github.com/fbiego/CST816S` (accessed Nov. 17, 2026).


- [6] NXP Semiconductors, “I2C-bus specification and user manual,” UM10204, Rev. 7.0, Oct. 2021.


- [7] *_Rotating electrical machines: Part 8: Terminal markings and direction of rotation_* , IEC 60034-8, 2007.


- [8] “Datasheet UMHA 63G-4 B3: Three phase electric motors, aluminum frame,” Inverter Drive Supermarket. [Online]. Available: `https\://inverterdrive.com/file/Datasheet-UMHA-63G-4-B3` (accessed Nov. 17, 2026).


- [9] NORVI, “ATV320 VFD control and monitoring over Modbus RTU,” NORVI application guide. [Online]. Available: `https\://github.com/NORVIControllers/NORVI-XVersion-02` (accessed Nov. 17, 2026).
