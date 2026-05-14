import streamlit as st
import pandas as pd
from streamlit_autorefresh import st_autorefresh

# Refresh every 10 seconds (10000 ms)
count = st_autorefresh(interval=10000, limit=None, key="data_refresh")

# Retriving the data from Google Sheets - Library Database
GOOGLE_SHEET_CSV = "https://docs.google.com/spreadsheets/d/17lmgOYsNALd3Q7lwNWvqmzuSnGsaE6nQch-l9HRbapQ/export?format=csv" 

@st.cache_data(ttl=5)
def load_data():
    df = pd.read_csv(GOOGLE_SHEET_CSV)
    df.columns = df.columns.str.strip()

    # Process availability based on status
    def get_availability(status):
        status_upper = str(status).upper()
        if status_upper == "MISMATCH":
            return "Available"
        elif status_upper == "CHECKED_OUT":
            return "Not Available"
        elif status_upper == "IN":
            return "Available"
        elif status_upper == "OUT":
            return "Available"

    df['Availability'] = df['Status'].apply(get_availability)
    return df

df = load_data()

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

# Main heading with heading and description
st.markdown(
    """
    <h1 style='text-align: center; font-family:sans-serif;'>Book Availability</h1>
    <p style='text-align: justify; font-size: 16px; color: #4B4B4B; font-family:sans-serif;'>
    Here you can check the current availability of books in the library. Each record shows
    the book name, author, genre, publication date, shelf and slot location, and current availability.
    Green indicates available books, and red indicates books that are currently checked out.
    </p>
    """,
    unsafe_allow_html=True
)

st.write("---")

# Tabular view of the book availability along with its information
def generate_table(df):
    table_html = "<div style='display:flex; justify-content:center;'><table style='width:100%; border-collapse: collapse; font-family:sans-serif; text-align:center;'>"
    
    # Table header
    table_html += "<tr>"
    headers = ["Book Name", "Author", "Genre", "Publication Date", "Shelf", "Slot", "Availability"]
    for h in headers:
        table_html += f"<th style='border: 1px solid #ddd; padding: 15px;'>{h}</th>"
    table_html += "</tr>"
    
    # Table rows
    for _, row in df.iterrows():
        table_html += "<tr>"
        table_html += f"<td style='border: 1px solid #ddd; padding: 15px;'>{row['Book Name']}</td>"
        table_html += f"<td style='border: 1px solid #ddd; padding: 15px;'>{row['Author']}</td>"
        table_html += f"<td style='border: 1px solid #ddd; padding: 15px;'>{row['Genre']}</td>"
        table_html += f"<td style='border: 1px solid #ddd; padding: 15px;'>{row['Publication Date']}</td>"
        table_html += f"<td style='border: 1px solid #ddd; padding: 15px;'>{row['Shelf']}</td>"
        table_html += f"<td style='border: 1px solid #ddd; padding: 15px;'>{row['Slot']}</td>"

        # Colour coded display of availability - red for unavailable and green for available
        color = "green" if row['Availability'] == "Available" else "red"
        table_html += f"<td style='border: 1px solid #ddd; padding: 15px; color:{color}; font-weight:bold;'>{row['Availability']}</td>"
        table_html += "</tr>"
    
    table_html += "</table></div>"
    return table_html

# Render table
st.markdown(generate_table(df), unsafe_allow_html=True)