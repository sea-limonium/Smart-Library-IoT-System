import streamlit as st

st.set_page_config(page_title="Library Dashboard", layout="wide")

# Initialize session state for login
if 'logged_in' not in st.session_state:
    st.session_state.logged_in = False

# Main heading with introductory text
st.markdown(
    """
    <h1 style='text-align: center; font-family:sans-serif;'>Welcome to the Library Dashboard!</h1>
    <p></p>
    <p style='text-align: center; font-size: 16px; font-family:sans-serif;'>
    Your one-stop place to check library updates, book availability, and peak hours.
    </p>
    """,
    unsafe_allow_html=True
)

st.write("---")

# Librarian login on side bar
st.sidebar.header("Librarian Login")

# Show login/logout based on state
if not st.session_state.logged_in:
    username = st.sidebar.text_input("Username") 
    password = st.sidebar.text_input("Password", type="password")

    if st.sidebar.button("Login"):
        if username == "admin" and password == "1234":   #need to change 
            st.session_state.logged_in = True
            st.sidebar.success("Login Successful!")
            st.rerun()
        else:
            st.sidebar.error("Invalid login")
else:
    st.sidebar.success("✓ Logged in as admin")
    if st.sidebar.button("Logout"):
        st.session_state.logged_in = False
        st.rerun()

# Dummy newsletter widgets
st.markdown("### Library Newsletter")
col1, col2, col3 = st.columns(3)

with col1:
    st.info("""📌 New arrivals this week!
            
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam pharetra lectus erat, feugiat dignissim felis congue vitae. Sed a diam malesuadabus nisl. """)
with col2:
    st.success("""📌 Reading event on Friday!
               
Nulla auctor nulla a dolor ornare, consectetur placerat neque imperdiet. Suspendisse sed nibh convallis, lobortis metus posuere, tempor nisi. """)
with col3:
    st.warning("""📌 Library maintenance on Saturday!
               
Mauris accumsan mi lorem, id convallis urna molestie id. Cras eget ligula dui. In facilisis nibh vel facilisis interdum.""")

st.write("---")

# Quick access cards to view library availability and book availability for students
cards = [
    {
        "title": "Want to check the availability of a book?",
        "button_text": "Check Book Availability",
        "key": "book_availability"
    },
    {
        "title": "Want to view library availability?",
        "button_text": "View Library Availability",
        "key": "library_availability"
    }
]

for card in cards:
    st.markdown(
        f"""
        <div style='
            border: 2px solid white;
            padding: 30px;
            border-radius: 15px;
            text-align: center;
            margin-bottom: 30px;
            background-color: rgba(255, 255, 255, 0.05);
            box-shadow: 2px 2px 12px rgba(0,0,0,0.05);
        '>
            <h3 style='font-family:sans-serif;'>{card["title"]}</h3>
        </div>
        """,
        unsafe_allow_html=True
    )
    
    if st.button(card["button_text"], key=card["key"], use_container_width=True):
        if card["key"] == "library_availability":
            st.switch_page("pages/library_availability.py")
        elif card["key"] == "book_availability":
            st.switch_page("pages/book_availability.py")
    
    st.write("") 