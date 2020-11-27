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
                        # String - Message
                        length = data_queue[1]
                        if len(data_queue) < length+2:
                            break
                        message = str(data_queue[2:2+length])[12:-2]
                        print(f"Message: {message}")
                        data_queue = data_queue[2+length:]
                    elif command == 1:
                        # String - Error
                        length = data_queue[1]
                        if len(data_queue) < length+2:
                            break
                        message = str(data_queue[2:2+length])[12:-2]
                        print(f"Error: {message}")
                        data_queue = data_queue[2+length:]
                    elif command == 2:
                        # Timestamped (short) int list
                        count = data_queue[1]
                        message_length = 1 + 1 + 4 + 2 * count
                        if len(data_queue) < message_length:
                            break
                        timestamp, = struct.unpack('<L', data_queue[2:6])
                        vals = struct.unpack(f'<{count}h', data_queue[6:message_length])
                        print(f"Got ints: {timestamp}    {vals}")
                        data_queue = data_queue[message_length:]
                    elif command == 3:
                        # Timestamped float list
                        count = data_queue[1]
                        message_length = 1 + 1 + 4 + 4 * count
                        if len(data_queue) < message_length:
                            break
                        timestamp, = struct.unpack('<L', data_queue[2:6])
                        vals = struct.unpack(f'<{count}f', data_queue[6:message_length])
                        #print(f"Got floats: {timestamp}    {vals}")
                        if (len(vals) == float_count):
                            ts.append(timestamp/1000)
                            ts = ts[1:]
                            for i in range(float_count):
                                rs[i].append(vals[i])
                                rs[i] = rs[i][1:]
                        data_queue = data_queue[message_length:]
                update_graph()
            await client.write_gatt_char(w_char, bytes('e', 'ascii'))
        except KeyboardInterrupt:
            # Exits loop. Client will disconnect.
            await client.write_gatt_char(w_char, bytes('e', 'ascii'))
            pass


addr = "48:87:2D:11:85:02"
loop = asyncio.get_event_loop()
loop.run_until_complete(communication(addr))
