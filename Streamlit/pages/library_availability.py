import streamlit as st
import pandas as pd
import plotly.express as px
from datetime import datetime
from streamlit_autorefresh import st_autorefresh

# Refresh every 10 seconds (10000 ms)
count = st_autorefresh(interval=10000, limit=None, key="data_refresh")

# Retrieving the data from Google Sheets - Gate System
PEAK_HOURS_CSV = "https://docs.google.com/spreadsheets/d/16Sxj_9a2VlhHgwZPhsQ5AC0hwujbgwMiR8YOlTME3_w/export?format=csv&gid=0"
COUNT_CSV = "https://docs.google.com/spreadsheets/d/16Sxj_9a2VlhHgwZPhsQ5AC0hwujbgwMiR8YOlTME3_w/export?format=csv&gid=601760551"

# Initialize session state for tracking changes
if 'last_timestamp' not in st.session_state:
    st.session_state.last_timestamp = None
if 'last_count' not in st.session_state:
    st.session_state.last_count = 0

# Loading the data from Google Sheets - for peak hours
@st.cache_data
def load_data():
    df = pd.read_csv(PEAK_HOURS_CSV)

    # Removing any spaces to avoid error
    df.columns = df.columns.str.strip().str.lower()
    
    # Convert timestamp column
    df['timestamp'] = pd.to_datetime(df['timestamp'], errors='coerce')

    # Remove rows where timestamp failed to parse
    df = df.dropna(subset=['timestamp'])

    # Generating required fields
    df['Day'] = df['timestamp'].dt.day_name()
    df['Hour'] = df['timestamp'].dt.hour 
    df['Date'] = df['timestamp'].dt.date

    return df

# Loading the data from Google Sheets - for library occupancy
@st.cache_data(ttl=10)  
def load_count_data():
    try:
        count_df = pd.read_csv(COUNT_CSV)
        count_df.columns = count_df.columns.str.strip().str.lower()
        
        # Convert timestamp column to datetime
        count_df['timestamp'] = pd.to_datetime(count_df['timestamp'], errors='coerce')
        
        # Remove rows where timestamp is not a time
        count_df = count_df.dropna(subset=['timestamp'])
        
        if count_df.empty:
            return None, 0
        
        # Fetch today's date
        today = pd.Timestamp.now().date()
        
        # Filter for today's records only
        today_records = count_df[count_df['timestamp'].dt.date == today]
        
        if today_records.empty:
            # No records for today, get the most recent record
            latest_row = count_df.iloc[-1]
            timestamp = latest_row['timestamp']
            count = int(latest_row['count'])
            return timestamp, count
        
        # Get the latest row from today's records
        latest_row = today_records.iloc[-1]
        timestamp = latest_row['timestamp']
        count = int(latest_row['count'])
        
        return timestamp, count
    except Exception as e:
        st.error(f"Error loading count data: {e}")
        return None, 0

# Load current count data and peak hour data
df = load_data()
current_timestamp, current_count = load_count_data()

# Check if data has changed
data_changed = False
if current_timestamp != st.session_state.last_timestamp:
    data_changed = True
    st.session_state.last_timestamp = current_timestamp
    st.session_state.last_count = current_count
else:
    current_count = st.session_state.last_count

current_in = current_count
current_available = 10 - current_in

# Back button to dashboard
if st.button("← Back to Main Dashboard"):
    st.switch_page("dashboard.py")
    
# Librarian login on side bar
st.sidebar.header("Librarian Login")
username = st.sidebar.text_input("Username") 
password = st.sidebar.text_input("Password", type="password")

if st.sidebar.button("Login"):
    if username == "admin" and password == "1234":   
        st.sidebar.success("Login Successful!")
    else:
        st.sidebar.error("Invalid login")

# Main heading
st.markdown(
    """
    <h1 style='text-align: center;'>Library Availability</h1>
    """,
    unsafe_allow_html=True
)

st.write("---")

# Current Library Occupancy Section
st.markdown(
    """
    <h2 style='text-align: center;'>Current Library Occupancy</h2>
    <p style='text-align: justify; font-size: 16px; color: #4B4B4B;'>
    This section provides an overview of the library’s current occupancy and capacity
    status using data collected from the gate systems. The visualisations
    update automatically, allowing you to monitor real-time activity, understand usage 
    trends, and anticipate busy periods. This helps in managing space efficiently while 
    ensuring a smooth and organised library experience for both staff and visitors.
    </p>
    """,
    unsafe_allow_html=True
)

# Max occupancy alert
if current_in >= 10:
    st.error("The library is currently at maximum occupancy (10/10)")

# Display occupancy in a full-width box
# Show if data was just updated
update_indicator = "🟢 Live" if data_changed else "⚪ Stable"

# Change border color based on occupancy
border_color = "#FF5252" if current_in >= 10 else "#4CAF50"

st.markdown(
    f"""
    <div style='
        border: 2px solid {border_color};
        border-radius: 10px;
        padding: 40px;
        text-align: center;
        background-color: #f9f9f9;
        box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        margin: 20px 0;
    '>
        <h3 style='color: #333; margin-bottom: 10px;'>Occupancy Status</h3>
        <p style='font-size: 12px; color: #888; margin-bottom: 30px;'>{update_indicator} • Last updated: {current_timestamp.strftime('%H:%M:%S') if current_timestamp else 'N/A'}</p>
        <div style='display: flex; justify-content: space-around; max-width: 600px; margin: 0 auto;'>
            <div>
                <p style='font-size: 20px; color: #666; margin-bottom: 10px;'>IN</p>
                <p style='font-size: 48px; font-weight: bold; color: {'#FF5252' if current_in >= 20 else '#4CAF50'}; margin: 0;'>{current_in}</p>
            </div>
            <div>
                <p style='font-size: 20px; color: #666; margin-bottom: 10px;'>AVAILABLE</p>
                <p style='font-size: 48px; font-weight: bold; color: {'#FF5252' if current_available == 0 else '#2196F3'}; margin: 0;'>{current_available}</p>
            </div>
        </div>
    </div>
    """,
    unsafe_allow_html=True
)

st.write("---")

# Peak Hours Section
st.markdown(
    """
    <h2 style='text-align: center;'>Peak Hours</h2>
    <p style='text-align: justify; font-size: 16px; color: #4B4B4B;'>
    This dashboard visualizes the peak hours of library usage based on entries recorded 
    by the gate system. You can select any weekday to see the busiest times, helping 
    in planning library visits. The data is automatically retrieved from the Google Sheets 
    integrated with the smart gate system, ensuring real-time updates.
    </p>
    """,
    unsafe_allow_html=True #allows html to be rendered
)
st.write("---")

days = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"]

selected_day = st.radio(
    "",
    days,
    horizontal=True
)

st.write("---")

# Default text shown when weekends is viewed
if selected_day in ["Saturday", "Sunday"]:
    st.warning(f"The library is closed on **{selected_day}**.")
    st.stop()

# Filter data based on the day
day_data = df[df["Day"] == selected_day]

if day_data.empty:
    st.warning("No data recorded for this day.")
    st.stop()

# Aggregrate hourly counts
hour_counts = (
    day_data.groupby("Hour")
    .size()
    .reset_index(name="Count")
)

# Limit hour range between 8–18
hour_counts = hour_counts[(hour_counts["Hour"] >= 8) & (hour_counts["Hour"] <= 18)]

# Converting hour to string such as "08:00", "09:00"
hour_counts['HourLabel'] = hour_counts['Hour'].apply(lambda x: f"{x:02d}:00")

# Plotting the bar graph
fig = px.bar(
    hour_counts,
    x="HourLabel", 
    y="Count",
    labels={"HourLabel": "", "Count": ""},
    title=f"Library Peak Hours – {selected_day}"
)

fig.update_layout(
    xaxis=dict(tickmode='linear')
)

st.plotly_chart(fig, width='stretch')