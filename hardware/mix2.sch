EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 4 5
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text HLabel 4300 3950 0    50   Input ~ 0
MIX2_IN
Text HLabel 6550 3650 2    50   Output ~ 0
MIX2_OUT
Wire Wire Line
	5150 3950 5150 4300
Connection ~ 5150 3950
$Comp
L Device:R R403
U 1 1 5FC2A268
P 5700 3400
F 0 "R403" H 5770 3446 50  0000 L CNN
F 1 "2k" H 5770 3355 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 5630 3400 50  0001 C CNN
F 3 "~" H 5700 3400 50  0001 C CNN
	1    5700 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 3550 5700 3650
$Comp
L Device:R R402
U 1 1 5FC2A427
P 5700 2900
F 0 "R402" H 5770 2946 50  0000 L CNN
F 1 "100" H 5770 2855 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 5630 2900 50  0001 C CNN
F 3 "~" H 5700 2900 50  0001 C CNN
	1    5700 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 3050 5700 3150
Wire Wire Line
	5700 3150 5450 3150
Wire Wire Line
	5150 3150 5150 3250
Connection ~ 5700 3150
Wire Wire Line
	5700 3150 5700 3250
$Comp
L power:GND #PWR0401
U 1 1 5FC2B3F7
P 5150 3250
F 0 "#PWR0401" H 5150 3000 50  0001 C CNN
F 1 "GND" H 5155 3077 50  0000 C CNN
F 2 "" H 5150 3250 50  0001 C CNN
F 3 "" H 5150 3250 50  0001 C CNN
	1    5150 3250
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C402
U 1 1 5FC2B545
P 5350 3150
F 0 "C402" V 5121 3150 50  0000 C CNN
F 1 "100nF" V 5212 3150 50  0000 C CNN
F 2 "Capacitor_THT:C_Disc_D3.0mm_W2.0mm_P2.50mm" H 5350 3150 50  0001 C CNN
F 3 "~" H 5350 3150 50  0001 C CNN
	1    5350 3150
	0    1    1    0   
$EndComp
Wire Wire Line
	5250 3150 5150 3150
$Comp
L power:GND #PWR0402
U 1 1 5FC2B912
P 5150 4700
F 0 "#PWR0402" H 5150 4450 50  0001 C CNN
F 1 "GND" H 5155 4527 50  0000 C CNN
F 2 "" H 5150 4700 50  0001 C CNN
F 3 "" H 5150 4700 50  0001 C CNN
	1    5150 4700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0403
U 1 1 5FC2BD58
P 5700 4700
F 0 "#PWR0403" H 5700 4450 50  0001 C CNN
F 1 "GND" H 5705 4527 50  0000 C CNN
F 2 "" H 5700 4700 50  0001 C CNN
F 3 "" H 5700 4700 50  0001 C CNN
	1    5700 4700
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 4600 5150 4700
$Comp
L Device:R R401
U 1 1 5FC2ADFE
P 5150 4450
F 0 "R401" H 5220 4496 50  0000 L CNN
F 1 "1M" H 5220 4405 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 5080 4450 50  0001 C CNN
F 3 "~" H 5150 4450 50  0001 C CNN
	1    5150 4450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 4600 5700 4700
$Comp
L Device:R R404
U 1 1 5FC2A836
P 5700 4450
F 0 "R404" H 5770 4496 50  0000 L CNN
F 1 "560" H 5770 4405 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 5630 4450 50  0001 C CNN
F 3 "~" H 5700 4450 50  0001 C CNN
	1    5700 4450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 4250 6100 4250
Connection ~ 5700 4250
Wire Wire Line
	5700 4250 5700 4300
$Comp
L Device:C_Small C403
U 1 1 5FC2CA7F
P 6200 4250
F 0 "C403" V 6325 4300 50  0000 C CNN
F 1 "100nF" V 6400 4325 50  0000 C CNN
F 2 "Capacitor_THT:C_Disc_D3.0mm_W2.0mm_P2.50mm" H 6200 4250 50  0001 C CNN
F 3 "~" H 6200 4250 50  0001 C CNN
	1    6200 4250
	0    1    1    0   
$EndComp
Wire Wire Line
	5700 3650 6100 3650
Connection ~ 5700 3650
Text HLabel 6550 4250 2    50   Input ~ 0
LO2
Wire Wire Line
	6300 4250 6550 4250
Wire Wire Line
	5700 2550 5700 2750
Text GLabel 5700 2550 1    50   Output ~ 0
VCC
Wire Wire Line
	5300 3950 5150 3950
$Comp
L dk_Transistors-FETs-MOSFETs-RF:2N3819_NonStock Q401
U 1 1 5F85B18C
P 5600 3950
F 0 "Q401" H 5788 4003 60  0000 L CNN
F 1 "2N3819" H 5788 3897 60  0000 L CNN
F 2 "digikey-footprints:TO-92-3" H 5800 4150 60  0001 L CNN
F 3 "https://my.centralsemi.com/get_document.php?cmp=1&mergetype=pd&mergepath=pd&pdf_id=2N3819.PDF" H 5800 4250 60  0001 L CNN
F 4 "2N3819CS-ND" H 5800 4350 60  0001 L CNN "Digi-Key_PN"
F 5 "2N3819" H 5800 4450 60  0001 L CNN "MPN"
F 6 "Discrete Semiconductor Products" H 5800 4550 60  0001 L CNN "Category"
F 7 "Transistors - FETs, MOSFETs - RF" H 5800 4650 60  0001 L CNN "Family"
F 8 "https://my.centralsemi.com/get_document.php?cmp=1&mergetype=pd&mergepath=pd&pdf_id=2N3819.PDF" H 5800 4750 60  0001 L CNN "DK_Datasheet_Link"
F 9 "/product-detail/en/central-semiconductor-corp/2N3819/2N3819CS-ND/4806873" H 5800 4850 60  0001 L CNN "DK_Detail_Page"
F 10 "TRANS JFET N-CH 25V 10MA TO-92" H 5800 4950 60  0001 L CNN "Description"
F 11 "Central Semiconductor Corp" H 5800 5050 60  0001 L CNN "Manufacturer"
F 12 "Active NonStock" H 5800 5150 60  0001 L CNN "Status"
	1    5600 3950
	1    0    0    -1  
$EndComp
Wire Wire Line
	4300 3950 5150 3950
$Comp
L Device:C_Small C401
U 1 1 5F9BABEF
P 6200 3650
F 0 "C401" V 6325 3700 50  0000 C CNN
F 1 "100nF" V 6400 3725 50  0000 C CNN
F 2 "Capacitor_THT:C_Disc_D3.0mm_W2.0mm_P2.50mm" H 6200 3650 50  0001 C CNN
F 3 "~" H 6200 3650 50  0001 C CNN
	1    6200 3650
	0    1    1    0   
$EndComp
Wire Wire Line
	6300 3650 6550 3650
$EndSCHEMATC
