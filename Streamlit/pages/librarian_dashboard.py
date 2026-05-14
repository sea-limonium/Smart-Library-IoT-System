import streamlit as st
import pandas as pd
from streamlit_autorefresh import st_autorefresh

# # Checking session status before allowing access to password protected librarian dashboard 
if 'logged_in' not in st.session_state or not st.session_state.logged_in:
    if st.button("← Go to Main Dashboard"):
        st.switch_page("dashboard.py")

    st.error("Authorization Required. You must be logged in to view this page.")
    st.info("Please log in from the main dashboard sidebar.")
    
    st.stop()

# If authenticated, show logout button in sidebar
st.sidebar.success("✓ Logged in as admin")
if st.sidebar.button("Logout", key="librarian_logout"):
    st.session_state.logged_in = False
    st.rerun()

# Refresh every 10 seconds (10000 ms)
count = st_autorefresh(interval=10000, limit=None, key="data_refresh")

# Retriving the data from Google Sheets - Logs & Inventory
LOGS_CSV = "https://docs.google.com/spreadsheets/d/17lmgOYsNALd3Q7lwNWvqmzuSnGsaE6nQch-l9HRbapQ/export?format=csv&gid=1656652695"
AVAILABILITY_CSV = "https://docs.google.com/spreadsheets/d/17lmgOYsNALd3Q7lwNWvqmzuSnGsaE6nQch-l9HRbapQ/export?format=csv&gid=0"

# Logging Data
@st.cache_data(ttl=5)
def load_data(url):
    df = pd.read_csv(url)
    df.columns = df.columns.str.strip().str.lower()
    if "timestamp" in df.columns:
        df['timestamp'] = pd.to_datetime(df['timestamp'], errors='coerce')
    return df

logs_df = load_data(LOGS_CSV)
summary_df = load_data(AVAILABILITY_CSV)

# Track previous mismatch UIDs for toast (pop up) notifs (following code little further down)
if "prev_mismatches" not in st.session_state:
    st.session_state.prev_mismatches = set()

# Main heading with description text
st.markdown(
    """
    <h1 style='text-align: center; font-family:sans-serif;'>Welcome, Adriana!</h1>
    <p style='text-align: center; font-size:16px; font-family:sans-serif;'>
    Here you can view the smart shelf logs and manage book availability. Scroll down to 
    see detailed entry logs, mismatches, pulled out, and placed back books.
    </p>
    """,
    unsafe_allow_html=True
)

st.write("---")

# Detailed log table from Log sheets
st.markdown("### Detailed Entry Logs of Smart Shelf")
if logs_df.empty:
    st.info("No detailed logs available.")
else:
    st.dataframe(logs_df, height=600)

st.write("---")

# Compute availability from Inventory sheet
if 'status' not in summary_df.columns:
    st.error("The summary sheet does not contain a 'status' column.")
    st.stop()

# Segregate summary tables
mismatch_df = summary_df[summary_df['status'].str.contains("MISMATCH", case=False, na=False)]
pulled_out_df = summary_df[summary_df['status'].str.upper() == "OUT"]
placed_back_df = summary_df[summary_df['status'].str.upper() == "IN"]
checkout_df = summary_df[summary_df['status'].str.upper() == "CHECKED_OUT"]

# Notif banner at top for mismatch 
if not mismatch_df.empty:
    for _, row in mismatch_df.iterrows():
        title = row.get("book name", "Unknown Title")
        correct_shelf = row.get("shelf", "?")
        correct_slot = row.get("slot", "?")
        curr_shelf = row.get("curr. shelf", "?")
        curr_slot = row.get("curr. slot", "?")

        st.markdown(
            f"""
            <div style="
                background-color:#2a0000;         /* dark red background */
                padding:15px;
                border:2px solid #ff4d4d;         /* full border outline */
                border-radius:6px;
                margin-bottom:10px;
                font-size:18px;
                font-family:sans-serif;
                color:#ffcccc;                    /* soft text */
            ">
                <strong style="color:#ff9999;">Mismatch Detected</strong><br>
                <strong>{title}</strong> was found in 
                <strong>Shelf {curr_shelf}, Slot {curr_slot}</strong>
                instead of 
                <strong>Shelf {correct_shelf}, Slot {correct_slot}</strong>.
            </div>
            """,
            unsafe_allow_html=True
        )

# Detects new mismatches compared to last refresh so it doesnt spam every 10 seconds
current_mismatches = set(mismatch_df['uid']) if 'uid' in mismatch_df.columns else set()

new_mismatches = current_mismatches - st.session_state.prev_mismatches

# Toast (pop up notifs) for new mismatches found
for uid in new_mismatches:
    book = mismatch_df[mismatch_df['uid'] == uid].iloc[0]

    title = book.get("book name", "Unknown Title")
    expected_shelf = book.get("shelf", "")
    expected_slot = book.get("slot", "")
    curr_shelf = book.get("curr. shelf", "")
    curr_slot = book.get("curr. slot", "")

    st.toast(
        f"Mismatch! '{title}' was found in Shelf {curr_shelf} Slot {curr_slot} "
        f"instead of Shelf {expected_shelf} Slot {expected_slot}",
        icon="⚠",
        duration=10
    )

# Update stored mismatches
st.session_state.prev_mismatches = current_mismatches

def render_table(df_table, title, empty_text):
    st.markdown(f"<h3 style='text-align:center; font-family:sans-serif;'>{title}</h3>", unsafe_allow_html=True)
    
    if df_table.empty:
        st.info(empty_text)
        st.write("---")
        return
    
    status_colours = {
        "IN": "green",
        "OUT": "goldenrod",
        "MISMATCH": "red",
        "CHECKED_OUT": "cyan"
    }
    
    # Centering the table with max-width
    table_html = """
    <div style='display:flex; justify-content:center;'>
        <table style='max-width:95%; width:auto; border-collapse: collapse; font-size:16px; text-align:center;'>
    """
    
    # Header
    table_html += "<tr>"
    for col in df_table.columns:
        table_html += f"<th style='border:1px solid #ddd; padding:15px;'>{col.title()}</th>"
    table_html += "</tr>"
    
    # Rows
    for _, row in df_table.iterrows():
        table_html += "<tr>"
        for col in df_table.columns:
            value = row[col]
            if col.lower() == "status":
                status_clean = str(value).strip().upper()
                color = "black"
                for key, c in status_colours.items():
                    if key == status_clean:
                        color = c
                        break
                table_html += f"<td style='border:1px solid #ddd; padding:15px; color:{color}; font-weight:bold;'>{value}</td>"
            else:
                table_html += f"<td style='border:1px solid #ddd; padding:15px;'>{value}</td>"

        table_html += "</tr>"
    
    table_html += "</table></div>"
    st.markdown(table_html, unsafe_allow_html=True)
    st.write("---")

# Render all tables
render_table(summary_df, "Overview of Book Inventory", "Error retrieving data.")
render_table(mismatch_df, "Books with Mismatch", "No books are mismatched.")
render_table(pulled_out_df, "Books Pulled Out", "No books have been pulled out.")
render_table(placed_back_df, "Books Placed Back", "No books have been placed back.")
render_table(checkout_df, "Books Checked Out", "No books are currently checked out.")