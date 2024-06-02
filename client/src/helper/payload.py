
# ASCII control characters
SOH = 1
ACK = 6
BEL = 7
CR  = 13
SO  = 14
SI  = 14
NAK = 21

# Buffer max size
BUFFSIZE = 250

# generate the payload from a message
def create_payload(code, msg):
    payload = code.to_bytes(1, 'little') + msg.encode()
    payload += checksum(payload) + SO.to_bytes(1, 'little')
    return payload

# parsing of the message in the payload
def parse_payload(payload, size):
    msg = payload[size:]
    return msg

# check if the payload is valid
def is_valid(payload, expected_code, size):
    if payload[0] != SOH and payload[-1] != CR \
       and payload[1] != expected_code:
        return False

    sum = 0
    for byte in payload[:-2]:
        sum += byte

    cs = sum if (sum < 256) else intmask(sum)

    return cs == payload[-2]

# take the less significant 8 bit
def intmask(val):
    lsb = bin(val & 0b11111111)
    return int(lsb, 2)

# generate the checksum from a package
def checksum(pkg):
    sum = 0
    for byte in pkg:
        sum += byte
    if (sum > 255):
        sum = intmask(sum)
    
    return sum.to_bytes(1, 'little')
