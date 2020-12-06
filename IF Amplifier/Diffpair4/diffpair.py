from scipy.optimize import fsolve
import numpy as np
from matplotlib import pyplot as plt

# Transistor characteristics
# Q1 is input transistor
# Q2 is output transistor
Vt: float = 26e-3  # thermal voltage
alpha = 0.99  # alpha of Q2
Is = 1e-12  # base-emitter reverse bias current of Q2

# Circuit characteristics
Vee = -9  # negative rail
Ree = 4700  # tail resistor
Vb2 = 0  # base voltage of Q2

# Model data
n = 500  # The number of data points
Vin_low = -20 * Vt
Vin_high = 20 * Vt

single_ended = True


def func_single_ended(ie, v_in):
    """ The set of equations which define the circuits behaviour.
    Calculates an error vector for the ie[0] and ie[1] estimates.
    ie[0] os the emitter current of Q1, ie[1] of Q2."""
    # Emitter voltage
    ve = Vee + (ie[0] + ie[1]) * Ree
    # Shockley equation for Q1
    z1 = ie[0] - Is / alpha * (np.exp((v_in - ve) / Vt) - 1)
    # Shockley equation for Q2
    z2 = ie[1] - Is / alpha * (np.exp((Vb2 - ve) / Vt) - 1)
    return [z1, z2]


# Use this version of func to drive the differential pair completely differentially.
# Compare the output with the circuits output when not driven differentially.

def func_diff(ie, v_in):
    """ The set of equations which define the circuits behaviour.
    Calculates an error vector for the ie[0] and ie[1] estimates.
    ie[0] os the emitter current of Q1, ie[1] of Q2."""
    # Emitter voltage
    ve = Vee + (ie[0] + ie[1]) * Ree
    # Shockley equation for Q1
    z1 = ie[0] - Is / alpha * (np.exp((v_in / 2 - ve) / Vt) - 1)
    # Shockley equation for Q2
    z2 = ie[1] - Is / alpha * (np.exp((-v_in / 2 - ve) / Vt) - 1)
    return [z1, z2]


# Input array in (A), returns an array in (mA)
def m_amps(i):
    return i * 1000


# Input array in (V), returns an array in (VT)
def v_thermal(v):
    return v / Vt


# Input voltage
Vin = np.linspace(Vin_low, Vin_high, n)

# Emitter currents
# ie[0] is emitter current for input transistor
# ie[1] is emitter current for output transistor
ie = np.empty((n, 2))

# Find initial guesses for the root
# Check the output when running this script. If fsolve() does
# not approach solutions soon enough, try better initial guesses!
guess = (-0.6 - Vee) / Ree / 2
root = np.array([guess, guess])

# Try to solve the system for each v_in. For the next v_in, use the
# solution of the previous v_in as an initial guess.
for i, v_in in enumerate(Vin):
    if single_ended:
        root = fsolve(func_single_ended, root, args=v_in)
    else:
        root = fsolve(func_diff, root, args=v_in)
    ie[i] = root

# Calculate maximum current gain.
# current gain is d(ie)/d(v_in)
gm = np.gradient(ie[:, 0], Vin)  # [S]
x = np.argmax(np.abs(gm))
gm_max = gm[x]
print("Maximum current gain gm={} [mS] at Vin={} [mV]".format(
    np.round(m_amps(-gm_max), 1),
    np.round(np.mean(Vin[x - 1:x + 1]) * 1000, 1)))

# check: https://www.youtube.com/watch?v=BG_Ny9Prx5s&t=1296s
# timestamp 21:36

# Calculate emitter and base-emitter voltage from emitter currents.
iee = ie[:, 0] + ie[:, 1]  # total tail current
ve = iee * Ree + Vee  # emitter voltage
vbe1 = Vin - ve  # Q1 base-emitter voltage

# calculate the small signal gain, at Vin ~ 0
x0 = int(-Vin_low / (Vin_high - Vin_low) * n)
# Vin[x0] ~ 0 (this assumes Vin_low<0 and Vin_high>0)
gm_calc = alpha * ie[x0, 1] / (2 * Vt)

# Plot it all, input voltage in [Vt], currents in [mA]

# Display gain
ax1 = plt.subplot(311)
plt.plot(v_thermal(Vin), 1000 * gm, '-', label='gm', color='brown')
plt.plot(v_thermal(Vin[x0]), 1000 * gm_calc, 'o', label='gm (calculated)', color='blue')
plt.setp(ax1.get_xticklabels(), visible=False)
plt.legend()
plt.grid(True)
ax1.set_ylabel('gm [mS]')
if single_ended:
    ax1.set_title("BJT differential pair driven single-ended")
else:
    ax1.set_title("BJT differential pair driven differentially")

# Display current
ax2 = plt.subplot(312)
plt.plot(v_thermal(Vin), m_amps(ie[:, 0]), 'g-', label='ie1')
plt.plot(v_thermal(Vin), m_amps(ie[:, 1]), 'b-', label='ie2')
# plt.plot(Vin / Vt, m_amps(ie2()), 'o', label='ie2')
plt.setp(ax2.get_xticklabels(), visible=False)
plt.legend()
plt.grid(True)
ax2.set_ylabel('ie1, ie2 [mA]')

# Display Vbe and Ve
ax3 = plt.subplot(313, sharex=ax2)
plt.plot(v_thermal(Vin), ve, 'r-', label='Ve')
plt.plot(v_thermal(Vin), vbe1, 'y-', label='Vbe1')
plt.legend()
plt.grid(True)
ax3.set_xlabel('Input voltage [Vt]')
ax3.set_ylabel('Ve, Vbe1 [V]')

fig = plt.gcf()
fig.set_size_inches(7, 10)
fig.canvas.set_window_title('IF diffpair')
fig.savefig("plot.png")

plt.show()
