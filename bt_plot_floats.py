import asyncio
import time
import bleak
from bleak import BleakClient
import struct
import matplotlib.pyplot as plt

float_count = 5

run = True

def on_close(event):
    global run
    run = False

fig = plt.figure()
fig.canvas.mpl_connect('close_event', on_close)
fig.canvas.set_window_title("Raw Readings - Bluetooth")
graphs = [fig.add_subplot(float_count, 1, i+1) for i in range(float_count)]
max_readings = 100
ts = [0] * max_readings
rs = [[0] * max_readings for _ in range(float_count)]
plots = [graphs[i].plot(ts, rs[i], color=f"C{i}")[0] for i in range(float_count)]
plt.ion()
plt.show()
data_queue = bytearray()

def update_graph():
    global float_count, ts, rs, plots, graphs, plt
    # Update graph
    for i in range(float_count):
        plots[i].set_xdata(ts)
        plots[i].set_ydata(rs[i])
        graphs[i].set_xlim([ts[0],ts[-1]])
        graphs[i].set_ylim([min(rs[i]), max(rs[i])])
    plt.draw()
    plt.pause(0.001)

def callback(sender: int, data: bytearray):
    global data_queue
    data_queue.extend(data)


async def communication(mac_addr: str):
    global graphs, ts, rs, plots, data_queue, run
    async with BleakClient(mac_addr) as client:
        try:
            svcs = await client.get_services()
            #service = svcs.get_service(svcs.services.keys()[0])
            w_char = svcs.get_characteristic(5)
            print(f"Write: {w_char}")
            r_char = svcs.get_characteristic(2)
            print(f"Read:  {r_char}")
            services = [i for i in svcs.services.keys()]
            characteristics = [i for i in svcs.characteristics.keys()]
            descriptors = [i for i in svcs.descriptors.keys()]
            print(f"Services:        {services}")
            print(f"Characteristics: {characteristics}")
            print(f"Descriptors:     {descriptors}")
            await client.start_notify(r_char, callback)
            await client.write_gatt_char(5, bytes('s', 'ascii'))
            while run:
                await client.write_gatt_char(w_char, bytes(' ', 'ascii'))
                time.sleep(0.05)

                # Deal with messages
                while len(data_queue) > 1:
                    command = data_queue[0]
                    if command == 0:
                        # String
                        length = data_queue[1]
                        if len(data_queue) < length+2:
                            break
                        message = str(data_queue[2:2+length])[2:-1]
                        data_queue = data_queue[2+length:]
                    elif command == 1:
                        # int
                        if len(data_queue) < 3:
                            break
                        i, = struct.unpack('<h', data_queue[1:3])
                        print(f"Got int: {i}")
                        data_queue = data_queue[3:]
                    elif command == 2:
                        # float
                        if len(data_queue) < 5:
                            break
                        f, = struct.unpack('<f', data_queue[1:5])
                        print(f"Got float: {f}")
                        data_queue = data_queue[5:]
                    elif command == 3:
                        # readings
                        if len(data_queue) < 17:
                            break
                        timestamp, ax, ay, az, gx, gy, gz = struct.unpack('<L6h', data_queue[1:17])
                        print(f"Got readings: {timestamp}    {ax:>6},{ay:>6},{az:>6}    {gx:>6},{gy:>6},{gz:>6}")
                        data_queue = data_queue[17:]
                    elif command == 4:
                        # readings
                        if len(data_queue) < 13:
                            break
                        timestamp, theta, tau = struct.unpack('<L2f', data_queue[1:13])
                        print(f"Got readings: {timestamp}    {theta:>6}    {tau:>6}")
                        data_queue = data_queue[13:]
                    elif command == 5:
                        # Stamped float list
                        length =data_queue[1]
                        if len(data_queue) < length*4 + 6:
                            break
                        timestamp, = struct.unpack('<L', data_queue[2:6])
                        vals = struct.unpack(f'<{length}f', data_queue[6:6+length*4])
                        #print(f"Floats {timestamp} - {vals}")
                        if length != float_count:
                            raise ValueError(f"Length: {length}, but expected {float_count}")
                        ts.append(timestamp/1000)
                        ts = ts[1:]
                        for i in range(float_count):
                            rs[i].append(vals[i])
                            rs[i] = rs[i][1:]
                        data_queue = data_queue[6+length*4:]
                update_graph()
            await client.write_gatt_char(w_char, bytes('e', 'ascii'))
        except KeyboardInterrupt:
            # Exits loop. Client will disconnect.
            await client.write_gatt_char(w_char, bytes('e', 'ascii'))
            pass


addr = "48:87:2D:11:85:02"
loop = asyncio.get_event_loop()
loop.run_until_complete(communication(addr))
