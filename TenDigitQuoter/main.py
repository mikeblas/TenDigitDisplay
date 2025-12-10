import time
from datetime import datetime, timezone

import yfinance as yf
import requests


# map a character to a bit pattern for the segments that spell it
letter_to_segment = {
    "0": 0b00111111,
    "1": 0b00000110,
    "2": 0b01011011,
    "3": 0b01001111,
    "4": 0b01100110,
    "5": 0b01101101,
    "6": 0b01111101,
    "7": 0b00000111,
    "8": 0b01111111,
    "9": 0b01100111,
    "A": 0b01110111,
    "C": 0b00111001,
    "D": 0b01011110,
    "E": 0b01111001,
    "F": 0b01110001,
    "L": 0b00111000,
    "S": 0b01101101,
    "U": 0b00011100,
    " ": 0,
    "=": 0b10001000
}

# "closed" in segments
closed = "39383F6d795e"

# Define the stock ticker symbol
ticker_symbol = "NVDA"
shares = 11124



# helper to decide if we are in UTC at the given time zone
def is_dst():
    t = time.localtime()
    return t.tm_isdst


def get_new_price():
    times = 1

    # Create a Ticker object
    ticker = None
    while ticker is None:
        if times == 1:
            print("Getting new price ...")
        else:
            print(f"   {times} tries")
        ticker = yf.Ticker(ticker_symbol)

    # Get the latest quote information
    # The .info attribute returns a dictionary with comprehensive data
    try:
        info = ticker.info
        current_price = info.get('currentPrice')
        company_name = info.get('longName')
        previous_close = info.get('previousClose')
        change = current_price - previous_close

        print(f"Company: {company_name}")
        print(f"Current Price : ${current_price:.2f}")
        print(f"Change        : ${change:.2f}")
        print(f"Value         : ${current_price * shares:.2f}")

        return current_price, change

    except Exception as e:
        print(f"An error occurred: {e}")


def market_open():
    now: datetime = datetime.now(timezone.utc)

    # not open weekends
    if now.weekday() in [5, 6]:
        return False

    # open from 1430 to 1700 in UTC
    open_hour = 13 if is_dst() else 14
    closed_hour = 20 if is_dst() else 21

    # we'll give five minutes on either side of that range
    if open_hour < now.hour <= closed_hour:
        return True
    if now.hour == closed_hour and now.minute < 5:
        return True
    if now.hour == open_hour and now.minute > 25:
        return True

    return False


last_price = None
last_change = None


def get_price():
    global last_price
    global last_change

    if last_price is None or market_open():
        last_price, last_change = get_new_price()

    return last_price, last_change


# compute the value string by multiplying the current price by the number of shares
# the string will show an integer total value, right justified in 10 spaces
def build_value_string(current_price):
    value_string = ("          " + str(int(current_price * shares)))[-10:]
    print(f'{value_string=}')
    total_value_string = ""
    for char in value_string:
        # print(f"{letter_to_segment[char]:02x}")
        total_value_string += f"{letter_to_segment[char]:02x}"

    return value_string, total_value_string


# the change string shows the current price, an up/down flag, and a change amount
# "ppp.ppucc.cc" is a total of ten characters
def build_change_string(current_price, change):
    # build the change string
    if change > 0:
        direction = "U"
    elif change < 0:
        direction = "D"
    else:
        direction = "="

    change_string = f"{current_price:5.2f}{direction}{abs(change):5.2f}"
    # print(f"{change_string}")
    total_change_string = ""
    i = 0
    while i < len(change_string):
        # get that character
        n = letter_to_segment[change_string[i]]
        i = i + 1

        # is the next character a decimal?
        if i < len(change_string) and change_string[i] == '.':
            n |= 0x80
            i = i + 1

        total_change_string += f"{n:02x}"

    return change_string, total_change_string


def try_tendigit_get(url):
    try:
        response = requests.get(url)
        if response.status_code != 200:
            print(response)
    except Exception as e:
        print(f"An error occurred putting to the display!: {e}")


panel_address = "192.168.0.183"

def main():
    while True:
        # get a new quote
        current_price, change = get_price()
        change = round(change, 2)
        print(f"{change=}")

        # build the value string
        value_string, total_value_string = build_value_string(current_price)

        # build the change_string
        change_string, total_change_string = build_change_string(current_price, change)

        # show the same quote for about 30 seconds
        until = time.time() + 30
        while time.time() < until:
            # show the change string
            url = f"http://{panel_address}/segments?{total_change_string}"
            print(f'displaying "{change_string}" with {url}')
            try_tendigit_get(url)
            # print(response.text)
            time.sleep(3)

            # show the value string
            url = f"http://{panel_address}/segments?{total_value_string}"
            print(f'displaying "{value_string}" with {url}')
            try_tendigit_get(url)
            # print(response.text)
            time.sleep(3)

            if not market_open():
                url = f"http://{panel_address}/segments?{closed}"
                print(f'displaying closed')
                try_tendigit_get(url)
                # print(response.text)
                time.sleep(3)

if __name__ == "__main__":
    main()

