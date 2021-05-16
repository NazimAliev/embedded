from django.urls import path
from . import views
from .views import PollsView
from .views import OnePollView
from .views import AnswersView 
from .views import AnswerTextView 
from .views import AnswerOneView 
from .views import AnswerChoiceView 

app_name = 'polls'

urlpatterns = [
	path('', views.index, name='index'),

	# GET requests

	# all polls, includins poll headers
	path('polls/', PollsView.as_view()),

	# concrete id poll and questions belong to the poll with choices of answer
	path('polls/poll_id=<int:poll_id>', OnePollView.as_view()),

	# user id poll answers (checked fields) with questions
	path('answers/poll_id=<int:poll_id>/user_id=<int:user_id>', AnswersView.as_view()),

	# POST requests: answers to questions

	path('answer_text/', AnswerTextView.as_view()),
	# question type = 0, POST body: question_id, user_id, answer_text

	path('answer_one/', AnswerOneView.as_view()),
	# question type = 1, POST body: user_id, choice (one) 
	
	path('answer_choice/', AnswerChoiceView.as_view()),
	# question type = 2, POST body: user_id, choice (one or multiple) 
]
